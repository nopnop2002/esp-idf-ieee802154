set(srcs "main.c" "ieee802154.c")

idf_component_register(SRCS ${srcs} INCLUDE_DIRS "." 
	REQUIRED_IDF_TARGETS esp32c5 esp32c6 esp32h2)
