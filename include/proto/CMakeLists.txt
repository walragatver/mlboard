include_directories(${PROTOBUF_INCLUDE_DIRS})
file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")

PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})
ADD_LIBRARY(proto ${ProtoHeaders} ${ProtoSources})
