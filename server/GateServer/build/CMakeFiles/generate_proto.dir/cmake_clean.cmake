file(REMOVE_RECURSE
  "CMakeFiles/generate_proto"
  "proto_generated/message.grpc.pb.cc"
  "proto_generated/message.grpc.pb.h"
  "proto_generated/message.pb.cc"
  "proto_generated/message.pb.h"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/generate_proto.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
