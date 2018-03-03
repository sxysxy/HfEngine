CXX_LIB = -lSDL2_image -lSDL2_net -ltiff -ljpeg -lpng -lz -lSDL2_ttf -lfreetype -lSDL2_mixer -lSDL2_test -lsmpeg2 -lvorbisfile -lvorbis -logg -lstdc++ -lSDL2 -lEGL -lGLESv1_CM -lGLESv2 -landroid 


EXEC = STGTest
OBJECTS = main.o \
          EasyRenderer.o
          
$(EXEC):$(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXEC) $(CXX_LIB)
  
main.o:
	$(CXX) -c main.cpp -o main.o 

EasyRenderer.o:
	$(CXX) -c EasyRenderer.cpp -o EasyRenderer.o 
	

          
	