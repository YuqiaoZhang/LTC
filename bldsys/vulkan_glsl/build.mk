all: \
	generated/cube.vert.inc generated/cube.frag.inc \
	generated/ltc.vert.inc generated/ltc.frag.inc \
	generated/ltc_mat.h generated/ltc_mag.h

generated/cube.vert.inc : shaders/cube.vert
	glslangValidator -g -Od -V shaders/cube.vert -x -o generated/cube.vert.inc

generated/cube.frag.inc : shaders/cube.frag
	glslangValidator -g -Od -V shaders/cube.frag -x -o generated/cube.frag.inc

generated/ltc.vert.inc : shaders/ltc.vert
	glslangValidator -g -Od -V shaders/ltc.vert -x -o generated/ltc.vert.inc

generated/ltc.frag.inc : shaders/ltc.frag
	glslangValidator -g -Od -V shaders/ltc.frag -x -o generated/ltc.frag.inc

generated/ltc_mat.h : assets/ltc_mat.dds
	./assets/include-bin  assets/ltc_mat.dds generated/ltc_mat.h

generated/ltc_mag.h : assets/ltc_mag.dds
	./assets/include-bin  assets/ltc_mag.dds generated/ltc_mag.h
