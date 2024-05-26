function(patch_csgo_protobufs)
    message("--[CSGO Protobuf] Patch CSGO Protobuf Files")

    #message("CSGO Protobufs are in SteamDatabase_Protobufs_SOURCE_DIR: ${steamdatabase_protobufs_SOURCE_DIR}")
    #message("Google Protobuf Descriptor is in Google Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}/google/protobuf/descriptor.proto")

    # Step 1: Copy the Protobuf Descriptor file
    message("--[CSGO Protobuf] 1. Copy the Protobuf Descriptor file from the Protobuf repository...")

    # Copy the descriptor.proto file from Protobuf_INCLUDE_DIRS to SteamDatabase_Protobufs_SOURCE_DIR
    set(DESCRIPTOR_FILE "${Protobuf_INCLUDE_DIRS}/google/protobuf/descriptor.proto")
    file(MAKE_DIRECTORY "${steamdatabase_protobufs_SOURCE_DIR}/google/protobuf")
    file(COPY "${DESCRIPTOR_FILE}" DESTINATION "${steamdatabase_protobufs_SOURCE_DIR}/google/protobuf")

    # Step 2: Define the patch content
    set(PATCH_CONTENT "syntax = \"proto2\";\n")

    # Step 3: Prepend syntax patch to every CSGO .proto file
    message("--[CSGO Protobuf] 2. Patch all CSGO protobuf files of the SteamDatabase_Protobufs repository...")
    file(GLOB PROTO_FILES "${steamdatabase_protobufs_SOURCE_DIR}/csgo/*.proto")

    foreach(PROTO_FILE ${PROTO_FILES})
        file(READ ${PROTO_FILE} PROTO_FILE_CONTENT)
        file(WRITE ${PROTO_FILE} "${PATCH_CONTENT}${PROTO_FILE_CONTENT}")
    endforeach()

    message("--[CSGO Protobuf] 3. Create a marker file to indicate that the patch has been applied.")
    file(TOUCH "${steamdatabase_protobufs_SOURCE_DIR}/csgo/patch_applied_marker")

endfunction()
