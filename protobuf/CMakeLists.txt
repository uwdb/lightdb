file(GLOB protos *.proto)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER "${protos}")
add_library(lightdb_protobuf ${PROTO_HEADER} ${PROTO_SRC})
