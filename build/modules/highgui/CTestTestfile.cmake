# CMake generated Testfile for 
# Source directory: /workspace/opencv/opencv-4.10.0/modules/highgui
# Build directory: /workspace/build/modules/highgui
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(opencv_test_highgui "/workspace/build/bin/opencv_test_highgui" "--gtest_output=xml:opencv_test_highgui.xml")
set_tests_properties(opencv_test_highgui PROPERTIES  LABELS "Main;opencv_highgui;Accuracy" WORKING_DIRECTORY "/workspace/build/test-reports/accuracy" _BACKTRACE_TRIPLES "/workspace/opencv/opencv-4.10.0/cmake/OpenCVUtils.cmake;1795;add_test;/workspace/opencv/opencv-4.10.0/cmake/OpenCVModule.cmake;1375;ocv_add_test_from_target;/workspace/opencv/opencv-4.10.0/modules/highgui/CMakeLists.txt;309;ocv_add_accuracy_tests;/workspace/opencv/opencv-4.10.0/modules/highgui/CMakeLists.txt;0;")
