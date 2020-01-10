file(GLOB protos *.proto)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER "${protos}")
add_library(lightdb_protobuf ${PROTO_HEADER} ${PROTO_SRC})
set_property(TARGET lightdb_protobuf PROPERTY POSITION_INDEPENDENT_CODE ON)

