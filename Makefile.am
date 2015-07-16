ACLOCAL_AMFLAGS = -I m4

if DSC
    DIR_DSC = dsc
endif
if EBB
    DIR_EBB = ebb
endif
if TB
   DIR_TB = tb
endif

SUBDIRS = $(DIR_DSC) $(DIR_EBB) $(DIR_TB)

dist_doc_DATA = ChangeLog.md

ChangeLog.md:
	$(am__cd) $(top_srcdir) && ./generate-changelog.sh \
	  > $(abs_top_builddir)/$@
