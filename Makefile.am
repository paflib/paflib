ACLOCAL_AMFLAGS = -I m4

if DSC
    DIR_DSC = dsc
endif
if EBB
    DIR_EBB = ebb
endif

SUBDIRS = $(DIR_DSC) $(DIR_EBB)

dist_doc_DATA = ChangeLog.md

ChangeLog.md:
	$(am__cd) $(top_srcdir) && ./generate-changelog.sh \
	  > $(abs_top_builddir)/$@
