/*******************************************************************
 ** SANE API
 *******************************************************************/

#define _GNU_SOURCE
#include "canon_pixma.h"
#include "sane.h"
#include "saneopts.h"
#include "sanei.h"
#include "sanei_backend.h"
#include <jpeglib.h>
#include <setjmp.h>
#include <unistd.h>
#include <math.h>

#ifdef __GNUC__
# define UNUSED(v) (void) v
#else
# define UNUSED(v)
#endif
#define	JPEGSCANBUFSIZE	(0x4000)	/* 16k */
#define min(A,B) (((A)<(B)) ? (A) : (B))
#define max(A,B) (((A)>(B)) ? (A) : (B))

#define PIXEL_TO_MM(pixels, dpi) SANE_FIX((double)pixels * 25.4 / (dpi))
#define MM_TO_PIXEL(millimeters, dpi) (SANE_Word)round(SANE_UNFIX(millimeters) * (dpi) / 25.4)

CNMSInt32 lastIOErrCode = 0;
CNMSInt32 lastBackendErrCode = 0;
CNMSInt32 lastModuleErrCode = 0;

static const char vendor_str[] = "CANON";
static const char type_str[] = "multi-function peripheral";

static const SANE_Device **dev_list = NULL;


typedef struct Handled{
	struct Handled * next;
	SGMP_Data_Lite sgmp;
	CANON_Device dev;
	CANON_ScanParam param;
	SANE_Option_Descriptor opt[NUM_OPTIONS];
	Option_Value val[NUM_OPTIONS];
	SANE_Range x_range;
	SANE_Range y_range;

	unsigned char * img_data;
	long img_size;
	long img_read;
	SANE_Bool cancel;
	SANE_Bool write_scan_data;
	SANE_Bool decompress_scan_data;
	SANE_Bool end_read;
}  canon_sane_t;

typedef struct {
	int			id;
	int			right;
	int			bottom;
} CIJSC_SIZE_TABLE;

enum{
	CIJSC_SCANMAIN_SCAN_FINISHED = 0,
	CIJSC_SCANMAIN_SCAN_CANCELED,
	CIJSC_SCANMAIN_SCAN_ERROR,
};

static const CIJSC_SIZE_TABLE sourceSize[] = {
	{ CIJSC_SIZE_CARD,		1074,  649 },		// Card
	{ CIJSC_SIZE_L_L,		1500, 1051 },		// L Landscape
	{ CIJSC_SIZE_L_P,		1051, 1500 },		// L Portrait
	{ CIJSC_SIZE_4X6_L,		1800, 1200 },		// 4"x6" Landscape
	{ CIJSC_SIZE_4X6_P,		1200, 1800 },		// 4"x6" Portrait
	{ CIJSC_SIZE_HAGAKI_L,	1748, 1181 },		// Hagaki Landscape
	{ CIJSC_SIZE_HAGAKI_P,	1181, 1748 },		// Hagaki Portrait
	{ CIJSC_SIZE_2L_L,		2102, 1500 },		// 2L Landscape
	{ CIJSC_SIZE_2L_P,		1500, 2102 },		// 2L Portrait
	{ CIJSC_SIZE_A5,		1748, 2480 },		// A5
	{ CIJSC_SIZE_B5,		2149, 3035 },		// B5
	{ CIJSC_SIZE_A4,		2480, 3507 },		// A4 size
	{ CIJSC_SIZE_LETTER,	2550, 3300 }		// Letter
};


static CIJSC_SIZE_TABLE
_get_source_size(int right, int bottom) {
   int x = 0;
   int obottom = -1;
   int oright = -1;
   static CIJSC_SIZE_TABLE maw_size = { CIJSC_SIZE_LETTER + 1, 2550, 3507};
   for (; x < CIJSC_SIZE_NUM; x++) {
      if (right > 0) {
         if ( sourceSize[x].right == right)
            return sourceSize[x];
         else if (oright == -1)
               oright = x;
         else if (sourceSize[oright].right > right &&
            sourceSize[x].right < sourceSize[oright].right)
               oright = x;
      }
      else if (bottom > 0) {
         if ( sourceSize[x].bottom == bottom)
            return sourceSize[x];
         else if (obottom == -1)
            obottom = x;
         else if (sourceSize[obottom].bottom > bottom &&
                  sourceSize[x].bottom < sourceSize[obottom].bottom)
               obottom = x;
      }
   }
   fprintf(stderr, "> %s => %d\n", (obottom != -1 ? "HAUTEUR" : "LARGEUR"), (obottom != -1 ? bottom : right));
   if (obottom == -1 && oright == -1) {
       fprintf(stderr, ">\t[%dx%d]\n",
               maw_size.right,
               maw_size.bottom);
       return maw_size;
   }
   fprintf(stderr, ">\t[%dx%d]\n",
               sourceSize[(obottom != -1 ? obottom : oright)].right,
               sourceSize[(obottom != -1 ? obottom : oright)].bottom);
   return sourceSize[(obottom != -1 ? obottom : oright)];
}

static CIJSC_SIZE_TABLE
_get_source_adf_size(int right, int bottom) {
   if (right > 0) {
      if ( sourceSize[CIJSC_SIZE_A4].right == right)
         return sourceSize[CIJSC_SIZE_A4];
      if ( sourceSize[CIJSC_SIZE_LETTER].right == right)
         return sourceSize[CIJSC_SIZE_LETTER];
      else if ( sourceSize[CIJSC_SIZE_A4].right < right)
         return sourceSize[CIJSC_SIZE_A4];
      else
         return sourceSize[CIJSC_SIZE_LETTER];
   }
   else if (bottom > 0) {
      if ( sourceSize[CIJSC_SIZE_A4].bottom == bottom)
         return sourceSize[CIJSC_SIZE_A4];
      else if ( sourceSize[CIJSC_SIZE_LETTER].bottom == bottom)
         return sourceSize[CIJSC_SIZE_LETTER];
      else if ( sourceSize[CIJSC_SIZE_LETTER].bottom < bottom)
         return sourceSize[CIJSC_SIZE_LETTER];
      else
         return sourceSize[CIJSC_SIZE_A4];
   }
   return sourceSize[CIJSC_SIZE_A4];
}

/* scanmode */
static const SANE_String_Const scan_table[] = {
        "Platen",
	"ADF Simplex",
        "ADF Duplex",
};

static const SANE_String_Const mode_list[] = {
  SANE_VALUE_SCAN_MODE_COLOR, SANE_VALUE_SCAN_MODE_GRAY,
  0
};

static const SANE_Int resbit_list[] =
{
	2, 300, 600
};

const char *canonJpegDataTmp = "/tmp/jpeg_canon.tmp";

static SANE_String_Const *
char_to_array(SANE_String_Const *tab,
                       int *tabsize,
                       SANE_String_Const mode) {
	SANE_String_Const *board = NULL;
	int i = 0;
	SANE_String_Const convert = NULL;

	if (mode == NULL)
		return (tab);

	convert = mode;

	for (i = 0; i < (*tabsize); i++) {
		if (strcmp(tab[i], mode) == 0)
		return (tab);
	}
	(*tabsize)++;
	if (*tabsize == 1)
		board = (SANE_String_Const *)malloc(sizeof(SANE_String_Const) * (*tabsize) + 1);
	else
		board = (SANE_String_Const *)realloc(tab, sizeof(SANE_String_Const) * (*tabsize) + 1);
	board[*tabsize - 1] = (SANE_String_Const)strdup(mode);
	board[*tabsize] = NULL;
	return (board);
}

static int
_get_source_num(const SANE_String_Const source)
{
    int i = 0;
    for (i = 0; scan_table[i]; ++i)
    {
	if (!strcmp(scan_table[i], source))
	   return i;
    }
    return CIJSC_SCANMODE_PLATEN;
}


static inline size_t max_string_size(const SANE_String_Const strings[])
{
    size_t max_size = 0;
    int i = 0;
    for (i = 0; strings[i]; ++i)
    {
        size_t size = strlen (strings[i]);
        if (size > max_size)
            max_size = size;
    }
    return max_size + 1;
}

SANE_Device*  convertFromCanonDev(const CANON_Device* cdev){
	SANE_Device* sdev = NULL;
	sdev = calloc(1, sizeof(SANE_Device));
	sdev->name = cdev->name;
	sdev->model = cdev->model;
	sdev->vendor = "CANON";
	sdev->type = "flatbed scanner";
	return sdev;
}

CMT_Status show_canon_cmt_error(CMT_Status status) {
	fprintf(stderr,"Error my backend :\t");
	switch(status) {
		case CMT_STATUS_UNSUPPORTED:
			fprintf(stderr,"Operation is not supported");
			break;
		case CMT_STATUS_CANCELLED:
			fprintf(stderr,"operation was cancelled");
			break;
		case CMT_STATUS_DEVICE_BUSY:
			fprintf(stderr,"device is busy; try again later");
			break;
		case CMT_STATUS_INVAL:
			fprintf(stderr,"data is invalid (includes no dev at open)");
			break;
		case CMT_STATUS_EOF:
			fprintf(stderr,"no more data available (end-of-file)");
			break;
		case CMT_STATUS_JAMMED:
			fprintf(stderr,"document feeder jammed");
			break;
		case CMT_STATUS_NO_DOCS:
			fprintf(stderr,"document feeder out of documents");
			break;
		case CMT_STATUS_COVER_OPEN:
			fprintf(stderr,"scanner cover is open");
			break;
		case CMT_STATUS_IO_ERROR:
			fprintf(stderr,"error during device I/O");
			break;
		case CMT_STATUS_NO_MEM:
			fprintf(stderr,"out of memory");
			break;
		case CMT_STATUS_ACCESS_DENIED:
			fprintf(stderr,"access to resource has been denied");
			break;
		default:
			break;
	}
	fprintf(stderr,"\n");
	return  status;
}

SANE_Status show_sane_cmt_error(CMT_Status status) {
  return (SANE_Status)show_canon_cmt_error(status);
}

struct my_error_mgr {
	struct jpeg_error_mgr errmgr;
	jmp_buf escape;
};

#define INPUT_BUFFER_SIZE	4096
typedef struct {
	struct jpeg_source_mgr pub;

	FILE *ctx;
	unsigned char buffer[INPUT_BUFFER_SIZE];
} my_source_mgr;
/*
 * Fill the input buffer --- called whenever buffer is emptied.
 */
static boolean fill_input_buffer (j_decompress_ptr cinfo)
{
	my_source_mgr * src = (my_source_mgr *) cinfo->src;
	int nbytes;

	nbytes = fread(src->buffer, 1, INPUT_BUFFER_SIZE, src->ctx);
	if (nbytes <= 0) {
		/* Insert a fake EOI marker */
		src->buffer[0] = (unsigned char) 0xFF;
		src->buffer[1] = (unsigned char) JPEG_EOI;
		nbytes = 2;
	}
	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;

	return TRUE;
}

/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */
static void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	my_source_mgr * src = (my_source_mgr *) cinfo->src;

	/* Just a dumb implementation for now.	Could use fseek() except
	 * it doesn't work on pipes.  Not clear that being smart is worth
	 * any trouble anyway --- large skips are infrequent.
	 */
	if (num_bytes > 0) {
		while (num_bytes > (long) src->pub.bytes_in_buffer) {
			num_bytes -= (long) src->pub.bytes_in_buffer;
			(void) src->pub.fill_input_buffer(cinfo);
			/* note we assume that fill_input_buffer will never
			 * return FALSE, so suspension need not be handled.
			 */
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.
 */
static void term_source (j_decompress_ptr cinfo)
{
	/* We don't actually need to do anything */
	return;
}
/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
static void init_source (j_decompress_ptr cinfo)
{
	/* We don't actually need to do anything */
	return;
}
/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */
static void jpeg_RW_src (j_decompress_ptr cinfo, FILE *ctx)
{
  my_source_mgr *src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  sizeof(my_source_mgr));
    src = (my_source_mgr *) cinfo->src;
  }

  src = (my_source_mgr *) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->ctx = ctx;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}


static void my_error_exit(j_common_ptr cinfo)
{
	struct my_error_mgr *err = (struct my_error_mgr *)cinfo->err;
	longjmp(err->escape, 1);
}

static void output_no_message(j_common_ptr cinfo)
{
	/* do nothing */
}
/* Load a JPEG type image from an SDL datasource */
CMT_Status canon_sane_decompress(canon_sane_t * handled,const char * filename)
{
	int start;
	struct jpeg_decompress_struct cinfo;
	JSAMPROW rowptr[1];
	unsigned char * surface = NULL;
	struct my_error_mgr jerr;
	FILE * src = NULL;
	int lineSize;
	src = fopen(filename,"rb");
	unlink(filename);
	start = ftell(src);


	/* Create a decompression structure and load the JPEG header */
	cinfo.err = jpeg_std_error(&jerr.errmgr);
	jerr.errmgr.error_exit = my_error_exit;
	jerr.errmgr.output_message = output_no_message;
	if(setjmp(jerr.escape)) {
		/* If we get here, libjpeg found an error */
		jpeg_destroy_decompress(&cinfo);
		if ( surface != NULL ) {
			free(surface);
		}
		return show_canon_cmt_error(CMT_STATUS_INVAL);
	}

	jpeg_create_decompress(&cinfo);
	jpeg_RW_src(&cinfo, src);
	jpeg_read_header(&cinfo, TRUE);


		/* Set 24-bit RGB output */
			cinfo.out_color_space =  JCS_RGB;
		cinfo.quantize_colors = FALSE;
		jpeg_calc_output_dimensions(&cinfo);
		/* Allocate an output surface to hold the image */
		surface = malloc(cinfo.output_width * cinfo.output_height * cinfo.output_components);

	if ( surface == NULL ) {
		jpeg_destroy_decompress(&cinfo);
		fseek(src, start, SEEK_SET);
		return show_canon_cmt_error(CMT_STATUS_NO_MEM);
	}

	lineSize = cinfo.output_width * cinfo.output_components;
	/* Decompress the image */
	jpeg_start_decompress(&cinfo);
	while ( cinfo.output_scanline < cinfo.output_height ) {
		rowptr[0] = (JSAMPROW)surface + (lineSize * cinfo.output_scanline);
		jpeg_read_scanlines(&cinfo, rowptr, (JDIMENSION) 1);
	}

	handled->img_data = surface;
	handled->img_size = lineSize*cinfo.output_height;
	handled->img_read = 0;
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(src);
	return CMT_STATUS_GOOD ;
}

void backend_error(SGMP_Data_Lite * data ,int *errCode){
	// set backend error code.
	*errCode = 0;
	CIJSC_get_backend_error_code(errCode );
	if( *errCode ) {
		DBGMSG("backend errCode = %d\n", *errCode );
		data->scan_result = CIJSC_SCANMAIN_SCAN_ERROR;
	}

	CIJSC_cancel();
	// delete scanned file.
}

CMT_Status canon_sane_read(canon_sane_t * handled){

	CMT_Status status = CMT_STATUS_GOOD;
	unsigned char* buf  = NULL;
	int readBytes = JPEGSCANBUFSIZE;
	int len = 0;
	FILE * file = NULL;
	int total =0;
	buf = (unsigned char *)calloc(JPEGSCANBUFSIZE,1);

	if(!buf){
		return show_canon_cmt_error(CMT_STATUS_NO_MEM);
	}

	file = fopen(canonJpegDataTmp,"wb");

	if(file == NULL){
		return show_canon_cmt_error(CMT_STATUS_INVAL);
	}

	while(status == CMT_STATUS_GOOD && !handled->cancel){
		/*Read data from the scanner, the data are send in jpeg format*/
		len = 0;
		readBytes = JPEGSCANBUFSIZE;
		status = CIJSC_read(buf,&readBytes);
		len = fwrite(buf,1,readBytes,file);
		total += len;
	}
	if(handled->cancel){
		status = CMT_STATUS_CANCELLED;
	}
	fclose(file);
	return status == CMT_STATUS_EOF ? CMT_STATUS_GOOD : status;
}

SANE_Status
sane_init (SANE_Int * version_code, SANE_Auth_Callback authorize)
{
	CMT_Status status;
	CNMSInt32 cnms_status = CNMS_NO_ERR;

	UNUSED (authorize);

	if (version_code != NULL)
		*version_code = SANE_VERSION_CODE(1, 0, 0);
	status = CIJSC_init((void*)NULL);
	if (status != CMT_STATUS_GOOD)
	{
		return show_sane_cmt_error(status);
	}

	cnms_status = KeepSettingCommonOpen();

	if(cnms_status != CNMS_NO_ERR ){
		/* show error dialog. */
		CIJSC_exit();
		return show_sane_cmt_error(CMT_STATUS_INVAL); //SANE_STATUS_INVAL;
	}

	return SANE_STATUS_GOOD;//status;
}



void
sane_exit (void){
	CIJSC_exit();
}


const CANON_Device ** canon_get_device(int* num_scan,CMT_Status* status){
	const CANON_Device **select_device_list = NULL;
	int i = 0;

	*status = CIJSC_get_devices( &select_device_list );
	if ( *status != CMT_STATUS_GOOD ) {
		CIJSC_exit();
		KeepSettingCommonClose();
		return NULL;
	}

	//nombre de scanner
	for(i=0; ; i++ ){
		if( select_device_list[i] == NULL ){
			break;
		}
	}

	*num_scan = i;
	if(i == 0){
		return NULL;
	}
	return select_device_list;
}

SANE_Status
sane_get_devices (const SANE_Device *** device_list, SANE_Bool local_only)
{
	UNUSED (local_only);
	int num_scan = 0;
	const CANON_Device ** canon_list = NULL;
	CMT_Status status;
	if (!device_list){
		return show_sane_cmt_error(CMT_STATUS_INVAL);
	}

	/* initialize selected device cache. */
	num_scan = 0;
	canon_list = canon_get_device(&num_scan,&status);
	if(canon_list == NULL){
		return show_sane_cmt_error(CMT_STATUS_NO_MEM);
	}
	if(status != CMT_STATUS_GOOD){
		return show_sane_cmt_error(status);
	}
	dev_list = (const SANE_Device **) calloc (num_scan + 1, sizeof (*dev_list));

	int i = 0;
	for(;i< num_scan;i++){
		dev_list[i] = convertFromCanonDev(canon_list[i]);
	        dev_list[i+1] = NULL;
	}

	*device_list = dev_list;
	return (dev_list) ? SANE_STATUS_GOOD : show_sane_cmt_error(CMT_STATUS_NO_MEM);
}

static CMT_Status init_canon_options(canon_sane_t * handled){
	unsigned long i;
	SGMP_Data_Lite * data = NULL;
	data = (SGMP_Data_Lite*)calloc(1,sizeof(SGMP_Data_Lite));
	if(!data){
		return show_canon_cmt_error(CMT_STATUS_NO_MEM);
	}

	data->scan_source = CIJSC_SOURCE_PHOTO;//DOCUMENT;
	data->scan_color = CIJSC_COLOR_COLOR;
	data->scan_format = CIJSC_FORMAT_JPEG;
	data->scan_size = CIJSC_SIZE_A4;
	data->scan_result = CIJSC_SCANMAIN_SCAN_FINISHED;
/*
	for ( i = 0; i < sizeof( sourceSize ) / sizeof( CIJSC_SIZE_TABLE ) ; i++ ) {
		if ( sourceSize[i].id == data->scan_size ) {

			break;
		}
	}
	if ( i == ( sizeof( sourceSize ) / sizeof( CIJSC_SIZE_TABLE ) ) ) {
		return show_canon_cmt_error(CMT_STATUS_INVAL);
	}
*/
	data->scan_res = resbit_list[1];
	data->scan_w = 2550;
        data->scan_h = 3507;
	data->scan_wx = data->scan_w;
	data->scan_hy = data->scan_h;
	data->scan_x = 0;
	data->scan_y = 0;
	data->scanning_page = 1;
	data->last_error_quit = CIJSC_ERROR_DLG_QUIT_FALSE;

	handled->sgmp= *data;
	SANE_Range x_range = {0,0,0},y_range = {0,0,0};
	x_range.min = 0;
	x_range.max = PIXEL_TO_MM(data->scan_w, 300); // data->scan_res;
	x_range.quant = 1;

	y_range.min = 0;
	y_range.max = PIXEL_TO_MM(data->scan_h, 300); // data->scan_res;
	y_range.quant = 1;

	handled->x_range = x_range;
	handled->y_range = y_range;
	return CMT_STATUS_GOOD;
}

static CMT_Status
init_options (canon_sane_t * s)
{
	int i;
	CMT_Status status = CMT_STATUS_GOOD;

	memset (s->opt, 0, sizeof (s->opt));
	memset (s->val, 0, sizeof (s->val));

	for (i = 0; i < NUM_OPTIONS; ++i)
	{
		s->opt[i].size = sizeof (SANE_Word);
		s->opt[i].cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;
	}

	status = init_canon_options(s);
	if(status != CMT_STATUS_GOOD){
		return show_canon_cmt_error(status);
	}


	s->opt[OPT_NUM_OPTS].title = SANE_TITLE_NUM_OPTIONS;
	s->opt[OPT_NUM_OPTS].desc = SANE_DESC_NUM_OPTIONS;
	s->opt[OPT_NUM_OPTS].type = SANE_TYPE_INT;
	s->opt[OPT_NUM_OPTS].cap = SANE_CAP_SOFT_DETECT;
	s->val[OPT_NUM_OPTS].w = NUM_OPTIONS;

	/* "Mode" group: */
	s->opt[OPT_MODE_GROUP].title = "Scan Mode";
	s->opt[OPT_MODE_GROUP].desc = "";
	s->opt[OPT_MODE_GROUP].type = SANE_TYPE_GROUP;
	s->opt[OPT_MODE_GROUP].cap = 0;
	s->opt[OPT_MODE_GROUP].constraint_type = SANE_CONSTRAINT_NONE;

	s->opt[OPT_MODE].name = SANE_NAME_SCAN_MODE;
	s->opt[OPT_MODE].title = SANE_TITLE_SCAN_MODE;
	s->opt[OPT_MODE].desc = SANE_DESC_SCAN_MODE;
	s->opt[OPT_MODE].type = SANE_TYPE_STRING;
	s->opt[OPT_MODE].unit = SANE_UNIT_NONE;
	s->opt[OPT_MODE].constraint_type = SANE_CONSTRAINT_STRING_LIST;
	s->opt[OPT_MODE].constraint.string_list = mode_list;
	s->val[OPT_MODE].s = strdup (mode_list[0]);
	s->sgmp.scan_color = (!strcmp(s->val[OPT_MODE].s, SANE_VALUE_SCAN_MODE_COLOR) ? CIJSC_COLOR_COLOR : CIJSC_COLOR_GRAY);

	s->opt[OPT_RESOLUTION].name = SANE_NAME_SCAN_RESOLUTION;
	s->opt[OPT_RESOLUTION].title = SANE_TITLE_SCAN_RESOLUTION;
	s->opt[OPT_RESOLUTION].desc = SANE_DESC_SCAN_RESOLUTION;
	s->opt[OPT_RESOLUTION].type = SANE_TYPE_INT;
	s->opt[OPT_RESOLUTION].unit = SANE_UNIT_DPI;
	/* TODO: Build the constraints on resolution in a smart way */
	s->opt[OPT_RESOLUTION].constraint_type = SANE_CONSTRAINT_WORD_LIST;
	s->opt[OPT_RESOLUTION].constraint.word_list = resbit_list;
	s->val[OPT_RESOLUTION].w = s->sgmp.scan_res;

	s->opt[OPT_PREVIEW].name = SANE_NAME_PREVIEW;
	s->opt[OPT_PREVIEW].title = SANE_TITLE_PREVIEW;
	s->opt[OPT_PREVIEW].desc = SANE_DESC_PREVIEW;
	s->opt[OPT_PREVIEW].cap = SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT;
	s->opt[OPT_PREVIEW].type = SANE_TYPE_BOOL;
	s->val[OPT_PREVIEW].w = SANE_FALSE;

	/* "Geometry" group: */
	s->opt[OPT_GEOMETRY_GROUP].title = "Geometry";
	s->opt[OPT_GEOMETRY_GROUP].desc = "";
	s->opt[OPT_GEOMETRY_GROUP].type = SANE_TYPE_GROUP;
	s->opt[OPT_GEOMETRY_GROUP].cap = SANE_CAP_ADVANCED;
	s->opt[OPT_GEOMETRY_GROUP].constraint_type = SANE_CONSTRAINT_NONE;

	/* top-left x */
	s->opt[OPT_TL_X].name = SANE_NAME_SCAN_TL_X;
	s->opt[OPT_TL_X].title = SANE_TITLE_SCAN_TL_X;
	s->opt[OPT_TL_X].desc = SANE_DESC_SCAN_TL_X;
	s->opt[OPT_TL_X].type = SANE_TYPE_FIXED;
	s->opt[OPT_TL_X].cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;
	s->opt[OPT_TL_X].unit = SANE_UNIT_MM;
	s->opt[OPT_TL_X].constraint_type = SANE_CONSTRAINT_RANGE;
	s->opt[OPT_TL_X].constraint.range = &s->x_range;
	s->val[OPT_TL_X].w = s->x_range.min;

	/* top-left y */

	s->opt[OPT_TL_Y].name = SANE_NAME_SCAN_TL_Y;
	s->opt[OPT_TL_Y].title = SANE_TITLE_SCAN_TL_Y;
	s->opt[OPT_TL_Y].desc = SANE_DESC_SCAN_TL_Y;
	s->opt[OPT_TL_Y].type = SANE_TYPE_FIXED;
	s->opt[OPT_TL_Y].cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;
	s->opt[OPT_TL_Y].unit = SANE_UNIT_MM;
	s->opt[OPT_TL_Y].constraint_type = SANE_CONSTRAINT_RANGE;
	s->opt[OPT_TL_Y].constraint.range = &s->y_range;
	s->val[OPT_TL_Y].w = s->y_range.min;

	/* bottom-right x */
	s->opt[OPT_BR_X].name = SANE_NAME_SCAN_BR_X;
	s->opt[OPT_BR_X].title = SANE_TITLE_SCAN_BR_X;
	s->opt[OPT_BR_X].desc = SANE_DESC_SCAN_BR_X;
	s->opt[OPT_BR_X].type = SANE_TYPE_FIXED;
	s->opt[OPT_BR_X].cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;
	s->opt[OPT_BR_X].unit = SANE_UNIT_MM;
	s->opt[OPT_BR_X].constraint_type = SANE_CONSTRAINT_RANGE;
	s->opt[OPT_BR_X].constraint.range = &s->x_range;
	s->val[OPT_BR_X].w = s->x_range.max;

	/* bottom-right y */
	s->opt[OPT_BR_Y].name = SANE_NAME_SCAN_BR_Y;
	s->opt[OPT_BR_Y].title = SANE_TITLE_SCAN_BR_Y;
	s->opt[OPT_BR_Y].desc = SANE_DESC_SCAN_BR_Y;
	s->opt[OPT_BR_Y].type = SANE_TYPE_FIXED;
	s->opt[OPT_BR_Y].cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;
	s->opt[OPT_BR_Y].unit = SANE_UNIT_MM;
	s->opt[OPT_BR_Y].constraint_type = SANE_CONSTRAINT_RANGE;
	s->opt[OPT_BR_Y].constraint.range = &s->y_range;
	s->val[OPT_BR_Y].w = s->y_range.max;

	/* OPT_SCAN_SOURCE */
        s->opt[OPT_SCAN_SOURCE].name = SANE_NAME_SCAN_SOURCE;
        s->opt[OPT_SCAN_SOURCE].title = SANE_TITLE_SCAN_SOURCE;
        s->opt[OPT_SCAN_SOURCE].desc = SANE_DESC_SCAN_SOURCE;
        s->opt[OPT_SCAN_SOURCE].type = SANE_TYPE_STRING;
        // s->opt[OPT_SCAN_SOURCE].size = x_source;
        s->opt[OPT_SCAN_SOURCE].cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;
        s->opt[OPT_SCAN_SOURCE].constraint_type = SANE_CONSTRAINT_STRING_LIST;
        s->opt[OPT_SCAN_SOURCE].constraint.string_list = scan_table;
        s->val[OPT_SCAN_SOURCE].s = strdup (scan_table[0]);
	s->sgmp.scan_scanmode = CIJSC_SCANMODE_PLATEN;

	return status == CMT_STATUS_GOOD ? status : show_canon_cmt_error(status);

}

SANE_Status
sane_open (SANE_String_Const name, SANE_Handle * h){

	canon_sane_t *  handled = NULL;
	CANON_Device dev;
	CMT_Status status = CMT_STATUS_INVAL;

	if(!name){
		return show_sane_cmt_error(CMT_STATUS_INVAL);
	}

	status = CIJSC_open2((char*)name,&dev);
	if(status != CMT_STATUS_GOOD){
		return show_sane_cmt_error(status);
	}


	handled = (canon_sane_t*)malloc(sizeof(canon_sane_t));
	if(!handled){
		return show_sane_cmt_error(CMT_STATUS_NO_MEM);
	}


	status = init_options(handled);
	if(status != CMT_STATUS_GOOD){
		return show_sane_cmt_error(status);
	}

	handled->dev = dev;
	handled->cancel = SANE_FALSE;
	handled->write_scan_data = SANE_FALSE;
	handled->decompress_scan_data = SANE_FALSE;
	handled->end_read = SANE_FALSE;
	*h = handled;
	return SANE_STATUS_GOOD;

}

void sane_cancel(SANE_Handle h){
	canon_sane_t* handled = h;
	handled->cancel = SANE_TRUE;
	CIJSC_cancel();
}

void
sane_close (SANE_Handle h){

	CIJSC_close( );
	if(h){
		free(h);
		h = NULL;
	}
	KeepSettingCommonClose();
	UNUSED(h);
}

const SANE_Option_Descriptor *
sane_get_option_descriptor(SANE_Handle h, SANE_Int n){
	canon_sane_t *s = h;
	if ((unsigned) n >= NUM_OPTIONS){
		return NULL;
	}
	return s->opt + n;
}

static int
_get_resolution(int resol)
{
    int x = 1;
    int n = 3;
    int old = -1;
    for (; x < n; x++) {
      if (resol == resbit_list[x])
         return resol;
      else if (resol < resbit_list[x])
      {
          if (old == -1)
             return resbit_list[1];
          else
             return old;
      }
      else
          old = resbit_list[x];
    }
    return old;
}


SANE_Status
sane_control_option (SANE_Handle h, SANE_Int n,
		     SANE_Action a, void *v, SANE_Int * i)
{
	canon_sane_t * handled = h;

	if (i)
		*i = 0;

	if (n >= NUM_OPTIONS || n < 0){
		return show_sane_cmt_error(CMT_STATUS_INVAL);
	}

	if(a == SANE_ACTION_GET_VALUE){
		switch(n){
			case OPT_NUM_OPTS:
			case OPT_RESOLUTION:
			case OPT_TL_X:
			case OPT_TL_Y:
			case OPT_BR_X:
			case OPT_BR_Y:
			case OPT_PREVIEW:
				*(SANE_Word *) v = handled->val[n].w;
				break;
            		case OPT_MODE:
			case OPT_SCAN_SOURCE:
                		strcpy (v, handled->val[n].s);
              			break;
            		case OPT_MODE_GROUP:
                		DBGMSG("unexpected get option, ignore");
			default:
                		break;
		}
		return SANE_STATUS_GOOD;
	}
	if(a == SANE_ACTION_SET_VALUE){
		int v1 = 0;
                CIJSC_SIZE_TABLE size;
		switch(n){
			case OPT_TL_X:
			case OPT_TL_Y:
			   	handled->val[OPT_TL_X].w = 0;
			   	handled->val[OPT_TL_Y].w = 0;
                                break;
			case OPT_BR_X:
			   	handled->val[n].w = *(SANE_Word *) v;
                                if (handled->sgmp.scan_scanmode == CIJSC_SCANMODE_PLATEN)
                                    size = _get_source_size(MM_TO_PIXEL(handled->val[n].w, 300), -1);
                                else
                                    size = _get_source_adf_size(MM_TO_PIXEL(handled->val[n].w, 300), -1);
			      	handled->sgmp.scan_wx = size.right * (handled->sgmp.scan_res == 300 ? 1 : 2);
                                handled->val[n].w = PIXEL_TO_MM(size.right, 300);
			      	handled->sgmp.scan_hy = size.bottom * (handled->sgmp.scan_res == 300 ? 1 : 2);
                                handled->val[OPT_BR_Y].w = PIXEL_TO_MM(size.bottom, 300);
			   	if(i){
			     		*i |= SANE_INFO_RELOAD_PARAMS | SANE_INFO_RELOAD_OPTIONS | SANE_INFO_INEXACT;
			   	}
			   	break;
			case OPT_BR_Y:
			   	handled->val[n].w = *(SANE_Word *) v;
                                if (handled->sgmp.scan_scanmode == CIJSC_SCANMODE_PLATEN)
                                    size = _get_source_size(-1, MM_TO_PIXEL(handled->val[n].w, 300));
                                else
                                    size = _get_source_adf_size(-1, MM_TO_PIXEL(handled->val[n].w, 300));
			      	handled->sgmp.scan_hy = size.bottom * (handled->sgmp.scan_res == 300 ? 1 : 2);
                                handled->val[n].w = PIXEL_TO_MM(size.bottom, 300);
			      	handled->sgmp.scan_wx = size.right * (handled->sgmp.scan_res == 300 ? 1 : 2);
                                handled->val[OPT_BR_X].w = PIXEL_TO_MM(size.right, 300);
			   	if(i){
			     		*i |= SANE_INFO_RELOAD_PARAMS | SANE_INFO_RELOAD_OPTIONS | SANE_INFO_INEXACT;
			   	}
			   	break;
			case OPT_PREVIEW:
			   	handled->val[n].w = *(SANE_Word *) v;
			      	if (handled->val[OPT_RESOLUTION].w == 600) {
			      	    handled->val[OPT_RESOLUTION].w = 300;
			      	    handled->sgmp.scan_res = 300 ;
			      	    handled->sgmp.scan_hy /= 2;
			      	    handled->sgmp.scan_wx /= 2;
                                    handled->val[OPT_BR_X].w = PIXEL_TO_MM(handled->sgmp.scan_wx, 300);
                                    handled->val[OPT_BR_Y].w = PIXEL_TO_MM(handled->sgmp.scan_hy, 300);
                                }
			   	if(i){
			     		*i |= SANE_INFO_RELOAD_PARAMS | SANE_INFO_RELOAD_OPTIONS | SANE_INFO_INEXACT;
			   	}
			   	break;
			case OPT_RESOLUTION:
			      	v1 = (int)*(SANE_Word*)v;
                              	v1 = _get_resolution(v1);
			      	handled->sgmp.scan_res = v1 ;
			      	if (handled->val[n].w != v1);
                                {
                                   if (handled->val[n].w > v1) {
			      	      handled->sgmp.scan_wx /= 2;
			              handled->sgmp.scan_hy /= 2;
                                   }
                                   else if (handled->val[n].w < v1) {
			      	      handled->sgmp.scan_wx *= 2;
			              handled->sgmp.scan_hy *= 2;
                                   }
                                   // handled->val[OPT_BR_X].w = PIXEL_TO_MM(handled->sgmp.scan_wx, 300);
                                   // handled->val[OPT_BR_Y].w = PIXEL_TO_MM(handled->sgmp.scan_hy, 300);
                                }
			      	handled->val[n].w = v1;
			      	if(i){
			        	*i |= SANE_INFO_RELOAD_PARAMS | SANE_INFO_RELOAD_OPTIONS | SANE_INFO_INEXACT;
			      	}
			   	break;
			case OPT_MODE:
                                if (handled->val[n].s)
                                    free(handled->val[n].s);
			   	handled->val[n].s = (SANE_Word *)strdup((char *)v);
			   	if(!strncasecmp(v,SANE_VALUE_SCAN_MODE_GRAY,3))
					handled->sgmp.scan_color = CIJSC_COLOR_GRAY;
			   	else
					handled->sgmp.scan_color = CIJSC_COLOR_COLOR;
			   	if(i){
			     		*i |= SANE_INFO_RELOAD_PARAMS | SANE_INFO_RELOAD_OPTIONS | SANE_INFO_INEXACT;
			   	}
			   	break;
			case OPT_SCAN_SOURCE:
                                if (handled->val[n].s)
                                    free(handled->val[n].s);
			   	handled->val[n].s = (SANE_Word *)strdup((char *)v);
			   	handled->sgmp.scan_scanmode = _get_source_num(v);
			   	if(i){
			     		*i |= SANE_INFO_RELOAD_PARAMS | SANE_INFO_RELOAD_OPTIONS | SANE_INFO_INEXACT;
			   	}
			   	break;
			default:
			   	break;
		}
	}

	return SANE_STATUS_GOOD;
}




SANE_Status
sane_start (SANE_Handle h){
	int errCode = 0;
	canon_sane_t* handled = h;
	CMT_Status status = CMT_STATUS_INVAL;
	CANON_ScanParam param;
	memset( &param, 0, sizeof(param) );


	param.XRes			= handled->sgmp.scan_res;
	param.YRes			= handled->sgmp.scan_res;
	param.Left			= 0;
	param.Top			= 0;
	param.Right			= handled->sgmp.scan_wx;
	param.Bottom			= handled->sgmp.scan_hy;
fprintf(stderr, "Res User  : [%d]\n", handled->sgmp.scan_res);
fprintf(stderr, "Format Max  : [0x0|%dx%d]\n", handled->sgmp.scan_w, handled->sgmp.scan_h);
fprintf(stderr, "Format User : [%dx%d|%dx%d]\n", handled->sgmp.scan_x, handled->sgmp.scan_y, handled->sgmp.scan_wx, handled->sgmp.scan_hy);
	param.ScanMode			= ( handled->sgmp.scan_color == CIJSC_COLOR_COLOR ) ? 4 : 2;
	param.ScanMethod		= ( handled->sgmp.scan_scanmode == CIJSC_SCANMODE_ADF_D_S ) ? CIJSC_SCANMODE_ADF_D_L : handled->sgmp.scan_scanmode;
	param.opts.p1_0			= 0;
	param.opts.p2_0			= 0;
	param.opts.p3_3			= 3;
	param.opts.DocumentType		= handled->sgmp.scan_source + 1;
	param.opts.p4_0			= 0;
	param.opts.p5_0			= 0;
	param.opts.p6_1			= 1;

	handled->param = param;


	handled->cancel = SANE_FALSE;
	handled->write_scan_data = SANE_FALSE;
	handled->decompress_scan_data = SANE_FALSE;
	handled->end_read = SANE_FALSE;
	handled->img_read = 0 ;

SCAN_START:
		/* scan start*/
	status = CIJSC_start( &param );
	if(status  != CMT_STATUS_GOOD ){
		handled->sgmp.last_error_quit = status;
		/* ADF : check status. */
		if ( param.ScanMethod != CIJSC_SCANMODE_PLATEN &&  status == CMT_STATUS_NO_DOCS ) {
			/* no paper */
			if( handled->sgmp.scanning_page == 1 ) {
				CIJSC_UI_error_show( &(handled->sgmp) );
				if(handled->sgmp.last_error_quit == CIJSC_VALUE_OK){
					goto SCAN_START;
				}
				else {
					/* scan canceled.*/
					/* delete disused file. */
					DBGMSG("CIJSC_cancel->\n");
					CIJSC_cancel();
				return show_sane_cmt_error(CMT_STATUS_NO_DOCS);
				}
			}else {
				/* delete disused file.*/
				DBGMSG("CIJSC_cancel->\n");
				CIJSC_cancel();
				CIJSC_close();
				CIJSC_open(handled->dev.name);
				return show_sane_cmt_error(CMT_STATUS_CANCELLED);
			}
		}
		DBGMSG("Error in CIJSC_start \n");
		backend_error(&(handled->sgmp),&errCode);
		return SANE_STATUS_CANCELLED;
	}

    /* get parameters */
    if((status = CIJSC_get_parameters(NULL)) != CMT_STATUS_GOOD){
      return SANE_STATUS_UNSUPPORTED;
    }

	return SANE_STATUS_GOOD;

}

SANE_Status
sane_get_parameters (SANE_Handle h, SANE_Parameters * p)//voir avec CIJSC_get_parameters
{

	canon_sane_t* handled = h;
	CANON_SCANDATA *scandata = NULL;
	scandata = (CANON_SCANDATA *)malloc(sizeof(CANON_SCANDATA));
	SANE_Parameters ps;

	int errCode = 0;
	ps.depth = 8;//8
	ps.last_frame = (handled->sgmp.scan_scanmode == CIJSC_SCANMODE_PLATEN ? SANE_TRUE : SANE_FALSE);
	ps.format = SANE_FRAME_RGB;
    	ps.pixels_per_line = handled->sgmp.scan_wx;
    	ps.lines = handled->sgmp.scan_hy;
    	ps.bytes_per_line = ps.pixels_per_line*3;
	*p = ps;

	return SANE_STATUS_GOOD;
}

SANE_Status
sane_read (SANE_Handle h, SANE_Byte * buf, SANE_Int maxlen, SANE_Int * len){
	canon_sane_t * handled = h;
	CMT_Status status = CMT_STATUS_GOOD;
	long readbyte;

	if(!handled|!buf|!len){
			return show_sane_cmt_error(CMT_STATUS_INVAL);
	}
	if(handled->cancel){
			return show_sane_cmt_error(CMT_STATUS_CANCELLED);
	}
	//lire toute les donnée du scanner les placé dans un fichier tmp
	if(!handled->write_scan_data){
		/*Get the jpeg image from the CANON's scanner
		 *the image always has 3 components.
		 *it will gray if the choosen mode is grayscale
		 */
		status = canon_sane_read(handled);
		if(status != CMT_STATUS_GOOD){
			return show_sane_cmt_error(status);
		}
		handled->write_scan_data = SANE_TRUE;
	}


	if(!handled->decompress_scan_data){
		/*Read the jpeg file and put the raw image in a buffer*/
		status = canon_sane_decompress(handled, canonJpegDataTmp);
		if(status != CMT_STATUS_GOOD){
			return show_sane_cmt_error(status);
		}

		handled->decompress_scan_data = SANE_TRUE;
	}
	if(handled->img_data == NULL){
			return show_sane_cmt_error(CMT_STATUS_INVAL);
	}

	if(!handled->end_read){
		/*read one scanline*/
		readbyte = min((handled->img_size - handled->img_read), maxlen);
		memcpy(buf,handled->img_data+handled->img_read,readbyte);
		handled->img_read += readbyte;
		*len = readbyte;
		if(handled->img_read == handled->img_size){
		/*if len > 0 and we are at end of file we need to do one more iteration to the total read bytes number*/
			handled->end_read = SANE_TRUE;
		}
		else if(handled->img_read > handled->img_size){
			/*if their is an error*/
			*len = 0;
			handled->end_read = SANE_TRUE;
			free(handled->img_data);
			handled->img_data = NULL;
			return show_sane_cmt_error(CMT_STATUS_INVAL);
		}
	}
	else{
		/*we need to have *len == 0 if status != SANE_STATUS_EOF*/
		*len = 0;
		free(handled->img_data);
		handled->img_data = NULL;
		return SANE_STATUS_EOF;
	}
	return SANE_STATUS_GOOD;
}

SANE_Status
sane_get_select_fd (SANE_Handle h, SANE_Int * fd)
{
	return SANE_STATUS_UNSUPPORTED;
}

SANE_Status
sane_set_io_mode(SANE_Handle handle,
			SANE_Bool non_blocking)
{
	return SANE_STATUS_UNSUPPORTED;
}
