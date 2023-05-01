pushd $(dirname $0) > /dev/null
glslc ./shader.frag -o frag.spv
glslc ./shader.vert -o vert.spv