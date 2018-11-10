### 编译过程
    cd build
    cmake ../src/
    make
    cd ../bin/
    %To run from camera
    ./tracker -p ../parameters.yml -tl
    %To run from file
    ./tracker -p ../parameters.yml -s ../datasets/06_car/car.mpg -tl
    %To init bounding box from file
    ./tracker -p ../parameters.yml -s ../datasets/06_car/car.mpg -b ../datasets/06_car/init.txt -tl
    %To train only in the firs frame (no tracking, no learning)
    ./tracker -p ../parameters.yml -s ../datasets/06_car/car.mpg -b ../datasets/06_car/init.txt 
    %To test the final detector (Repeat the video, first time learns, second time detects)
    ./tracker -p ../parameters.yml -s ../datasets/06_car/car.mpg -b ../datasets/06_car/init.txt -tl -r
