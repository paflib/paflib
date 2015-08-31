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
if PPR
  DIR_PPR = ppr
endif

SUBDIRS = common $(DIR_DSC) $(DIR_EBB) $(DIR_TB) $(DIR_PPR)

dist_doc_DATA = ChangeLog.md

ChangeLog.md:
	$(am__cd) $(top_srcdir) && ./generate-changelog.sh \
	  > $(abs_top_builddir)/$@
