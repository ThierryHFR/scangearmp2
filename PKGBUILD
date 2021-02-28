# Maintainer: fft <anonim288@gmail.com>
pkgname=scangearmp2-sane
pkgver=4.12.r54.g99e3e20
pkgrel=1
pkgdesc="Canon ScanGear MP v2 scanner utility and sane backend"
arch=('x86_64')
url="https://github.com/fftmp/scangearmp2"
license=('GPL' 'custom:canon')
depends=('gtk2')
makedepends=('cmake' 'libjpeg' 'sane')
provides=('scangearmp2')
conflicts=('scangearmp2')


pkgver() {
  cd ${startdir}
  git describe --tags --long | sed -r 's/^2_//; s/^([[:digit:]]+)(-|.)([[:digit:]]+)(-|.)([[:digit:]]+)/\1.\3.\5/; s/-/.r/; s/-/./g'
}

_builddir="${startdir}/build"

build() {
  mkdir -p ${_builddir}
  rm -rf ${_builddir}
  msg "Starting build ..."
  cmake -H"${_builddir}/.." -B${_builddir} -DCMAKE_INSTALL_PREFIX="/"
  make -j4 -C${_builddir}
}

package() {
  rm -rf ${pkgdir}
  make -C${_builddir} install DESTDIR=${pkgdir}
  #kostyl': /usr/sbin /usr/lib64 are symlinks in archlinux
  mkdir -p ${pkgdir}/usr/bin ${pkgdir}/usr/lib
  mv ${pkgdir}/usr/bin/*  ${pkgdir}/usr/bin/
  mv ${pkgdir}/usr/lib64/* ${pkgdir}/usr/lib/
  rmdir ${pkgdir}/usr/sbin ${pkgdir}/usr/lib64

  mkdir -p ${pkgdir}/usr/share/licenses/scangearmp2-sane
  mv ${pkgdir}/usr/share/scangearmp2/doc/* ${pkgdir}/usr/share/licenses/scangearmp2-sane/
  rmdir ${pkgdir}/usr/share/scangearmp2/doc/
}
