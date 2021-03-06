SET(MDOC3_PAGES
  libzip.mdoc
  zip_add.mdoc
  zip_add_dir.mdoc
  zip_close.mdoc
  zip_delete.mdoc
  zip_dir_add.mdoc
  zip_discard.mdoc
  zip_error_clear.mdoc
  zip_error_get.mdoc
  zip_error_get_sys_type.mdoc
  zip_error_to_str.mdoc
  zip_errors.mdoc
  zip_fclose.mdoc
  zip_fdopen.mdoc
  zip_file_add.mdoc
  zip_file_extra_field_delete.mdoc
  zip_file_extra_field_get.mdoc
  zip_file_extra_field_set.mdoc
  zip_file_extra_fields_count.mdoc
  zip_file_get_comment.mdoc
  zip_file_rename.mdoc
  zip_file_set_comment.mdoc
  zip_file_strerror.mdoc
  zip_fopen.mdoc
  zip_fopen_encrypted.mdoc
  zip_fread.mdoc
  zip_get_archive_comment.mdoc
  zip_get_archive_flag.mdoc
  zip_get_file_comment.mdoc
  zip_get_name.mdoc
  zip_get_num_entries.mdoc
  zip_get_num_files.mdoc
  zip_name_locate.mdoc
  zip_open.mdoc
  zip_rename.mdoc
  zip_set_archive_comment.mdoc
  zip_set_archive_flag.mdoc
  zip_set_default_password.mdoc
  zip_set_file_comment.mdoc
  zip_set_file_compression.mdoc
  zip_source_buffer.mdoc
  zip_source_file.mdoc
  zip_source_filep.mdoc
  zip_source_free.mdoc
  zip_source_function.mdoc
  zip_source_zip.mdoc
  zip_stat.mdoc
  zip_stat_init.mdoc
  zip_unchange.mdoc
  zip_unchange_all.mdoc
  zip_unchange_archive.mdoc
)
SET(MDOC1_PAGES
  zipcmp.mdoc
  zipmerge.mdoc
  ziptorrent.mdoc
)
SET(MDOC_PAGES ${MDOC1_PAGES} ${MDOC3_PAGES})
STRING(REGEX REPLACE .mdoc "" MAN1_PAGES "${MDOC1_PAGES}")
STRING(REGEX REPLACE .mdoc "" MAN3_PAGES "${MDOC3_PAGES}")
SET(MAN_PAGES ${MAN1_PAGES} ${MAN3_PAGES})

IF(CMAKE_SYSTEM_NAME MATCHES BSD)
  SET(MANFMT mdoc)
ELSE(CMAKE_SYSTEM_NAME MATCHES BSD)
  SET(MANFMT man)
ENDif(CMAKE_SYSTEM_NAME MATCHES BSD)

# installation

SET(MAN_PATH man)

FOREACH(MAN_PAGE ${MAN1_PAGES})
  INSTALL(FILES ${MAN_PAGE}.${MANFMT} DESTINATION ${MAN_PATH}/man1
    RENAME ${MAN_PAGE}.1)
ENDFOREACH(MAN_PAGE ${PROGRAMS})
FOREACH(MAN_PAGE ${MAN3_PAGES})
  INSTALL(FILES ${MAN_PAGE}.${MANFMT} DESTINATION ${MAN_PATH}/man3
    RENAME ${MAN_PAGE}.3)
ENDFOREACH(MAN_PAGE ${PROGRAMS})
# XXX: usually symlinks, but I only know how to do this way
INSTALL(FILES zip_error_clear.${MANFMT} DESTINATION ${MAN_PATH}/man3
  RENAME zip_file_error_clear.3)
INSTALL(FILES zip_error_get.${MANFMT} DESTINATION ${MAN_PATH}/man3
  RENAME zip_file_error_get.3)
INSTALL(FILES zip_file_add.${MANFMT} DESTINATION ${MAN_PATH}/man3
  RENAME zip_file_replace.3)
INSTALL(FILES zip_file_extra_field_delete.${MANFMT} DESTINATION ${MAN_PATH}/man3
  RENAME zip_file_extra_field_delete_by_id.3)
INSTALL(FILES zip_file_extra_field_get.${MANFMT} DESTINATION ${MAN_PATH}/man3
  RENAME zip_file_extra_field_get_by_id.3)
INSTALL(FILES zip_file_extra_fields_count.${MANFMT} DESTINATION ${MAN_PATH}/man3
  RENAME zip_file_extra_fields_count_by_id.3)
INSTALL(FILES zip_fopen.${MANFMT} DESTINATION ${MAN_PATH}/man3
  RENAME zip_fopen_index.3)
INSTALL(FILES zip_fopen_encrypted.${MANFMT} DESTINATION ${MAN_PATH}/man3
  RENAME zip_fopen_index_encrypted.3)
INSTALL(FILES zip_add.${MANFMT} DESTINATION ${MAN_PATH}/man3
  RENAME zip_replace.3)
INSTALL(FILES zip_stat.${MANFMT} DESTINATION ${MAN_PATH}/man3
  RENAME zip_stat_index.3)
INSTALL(FILES zip_file_strerror.${MANFMT} DESTINATION ${MAN_PATH}/man3
  RENAME zip_strerror.3)

# custom maintainer targets

#FOREACH(MAN ${MAN_PAGES})
#  ADD_CUSTOM_TARGET(${MAN}.man
#    DEPENDS ${MAN}.mdoc
#    COMMAND mdoc2man ${CMAKE_CURRENT_SOURCE_DIR}/${MAN}.mdoc > ${MAN}.new
#    COMMAND diff -I NiH ${MAN}.new ${CMAKE_CURRENT_SOURCE_DIR}/${MAN}.man || mv ${MAN}.new ${CMAKE_CURRENT_SOURCE_DIR}/${MAN}.man
#    COMMAND rm -f ${MAN}.new
#    COMMENT Updating man page ${MAN}.man
#    )
#  ADD_CUSTOM_TARGET(${MAN}.html
#    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${MAN}.mdoc
#    COMMAND nroff -mdoc2html ${CMAKE_CURRENT_SOURCE_DIR}/${MAN}.mdoc | sed -e "s,../html[0-9]/,," > ${MAN}.html
#    COMMENT Updating HTML page ${MAN}.html
#    )
#ENDFOREACH(MAN)

#STRING(REGEX REPLACE .mdoc .man MANDEPEND "${MDOC_PAGES}")
#ADD_CUSTOM_TARGET(update-man
#  DEPENDS ${MANDEPEND})

#STRING(REGEX REPLACE .mdoc .html HTMLDEPEND "${MDOC_PAGES}")
#ADD_CUSTOM_TARGET(update-html
#  DEPENDS ${HTMLDEPEND})

# XXX: strange error:
#[ 94%] zipcmp.html
#[ 97%] Built target zipcmp.html
#[ 97%] zipmerge.html
#[100%] Built target zipmerge.html
#make: don't know how to make zipcmp.html. Stop

# XXX: why are all the man pages rebuilt on a second 'make update-html'?
