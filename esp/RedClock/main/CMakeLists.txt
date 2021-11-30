idf_component_register(SRCS "clockMain.c" "clockTZ.c" "clock7seg.c"
                        "clockSntp.c" "clockGps.c" "clockBme280.c"
                        "clockspiffs.c" "clockServer.c" "clockConnect.c"
                        "clockHtml.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES
#                        "spiffsdata/favicon.ico"
                        "upload_script.html"
# adds a trailing nul? makes it work?                        
                    EMBED_TXTFILES
                        "certs/cacert.pem"
                        "certs/prvtkey.pem"
#                        "spiffsdata/masonry.pkgd.min.js"
#                        "spiffsdata/jquery-3.5.1.min.js"
                    )

# https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html
spiffs_create_partition_image(storage spiffsdata FLASH_IN_PROJECT)

