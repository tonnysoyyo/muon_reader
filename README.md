# muon_reader

Code to plot some muon plots using icecube/corsika_reader

Follow the instructions for icecube/corsika_reader, once done, run this code like this:

export LD_LIBRARY_PATH=/lib:$LD_LIBRARY_PATH

g++ -o corsika_root_plot ../main.cpp     `root-config --cflags --libs`     -I/corsika_reader/include     -L/lib -lCorsikaReader

./corsika_root_plot /path/to/data