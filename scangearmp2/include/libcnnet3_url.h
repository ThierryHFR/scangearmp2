/*
 *  Canon Inkjet Printer Driver for Linux
 *  Copyright CANON INC. 2014-2016
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * NOTE:
 *  - As a special exception, this program is permissible to link with the
 *    libraries released as the binary modules.
 *  - If you write modifications of your own for these programs, it is your
 *    choice whether to permit this exception to apply to your modifications.
 *    If you do not wish that, delete this exception.
*/

#ifndef __INC_CNNET3_URL__
#define __INC_CNNET3_URL__

#define CNNET3_URL_MASTER     "/canon/ij/command1/port1"
#define CNNET3_URL_CONTROL    "/canon/ij/command2/port1"
#define CNNET3_URL_RESOURCE   "/canon/ij/command2/port2"
#define CNNET3_URL_SCAN       "/canon/ij/command2/port3"
#define CNNET3_URL_PRINT      "/canon/ij/command2/port4"
#define CNNET3_URL_CONTROL_FAX  "/canon/ij/command2/port5"
#define CNNET3_URL_EVENT      "/canon/ij/command1/port2"

#define CNNET3_DEFAULT_RW_TIMEOUT_MASTER 3000
#define CNNET3_DEFAULT_RW_TIMEOUT_CONTROL 3000
#define CNNET3_DEFAULT_RW_TIMEOUT_RESOURCE 20000
#define CNNET3_DEFAULT_RW_TIMEOUT_SCAN 3000
#define CNNET3_DEFAULT_RW_TIMEOUT_PRINT 3000
#define CNNET3_DEFAULT_RW_TIMEOUT_EVENT 3000
#define CNNET3_DEFAULT_RW_TIMEOUT_CONTROL_FAX   3000

#endif
