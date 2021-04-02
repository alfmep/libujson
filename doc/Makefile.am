#
# Copyright (C) 2017,2020 Dan Arrhenius <dan@ultramarin.se>
#
# This file is part of ujson.
#
# ujson is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

if HAVE_DOXYGEN

directory = $(srcdir)/../doc/html

#dist_pkgdata_DATA = $(directory)
#dist_pkgdata_DATA = $(shell find $(directory) -type f)
$(directory): doxyfile.stamp

doxyfile.stamp:
	$(DOXYGEN) doxygen.cfg
	echo Timestamp > doxyfile.stamp


install-data-local:
#       Install HTML documentation
	$(MKDIR_P) $(DESTDIR)/$(docdir)/html/search
	$(INSTALL_DATA) html/*.html $(DESTDIR)/$(docdir)/html
	$(INSTALL_DATA) html/*.png $(DESTDIR)/$(docdir)/html
if HAVE_DOT
	$(INSTALL_DATA) html/*.md5 $(DESTDIR)/$(docdir)/html
endif
	$(INSTALL_DATA) html/*.css $(DESTDIR)/$(docdir)/html
	$(INSTALL_DATA) html/*.js $(DESTDIR)/$(docdir)/html
#       Install man pages
	$(MKDIR_P) $(DESTDIR)/$(mandir)/man3
	$(INSTALL_DATA) man/man3/ujson* $(DESTDIR)/$(mandir)/man3


CLEANFILES = doxyfile.stamp

all-local: doxyfile.stamp
clean-local:
	rm -rf $(top_srcdir)/doc/html
	rm -rf $(top_srcdir)/doc/man
	rm -f $(top_srcdir)/doc/doxyfile.stamp

uninstall-local:
	rm -rf $(DESTDIR)/$(docdir)/html
	rm -f $(DESTDIR)/$(mandir)/man3/ujson*

endif
