# CMake generated Testfile for 
# Source directory: /workspace/opencv/opencv-4.10.0/modules/video
# Build directory: /workspace/build/modules/video
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(opencv_test_video "/workspace/build/bin/opencv_test_video" "--gtest_output=xml:opencv_test_video.xml")
set_tests_properties(opencv_test_video PROPERTIES  LABELS "Main;opencv_video;Accuracy" WORKING_DIRECTORY "/workspace/build/test-reports/accuracy" _BACKTRACE_TRIPLES "/workspace/opencv/opencv-4.10.0/cmake/OpenCVUtils.cmake;1795;add_test;/workspace/opencv/opencv-4.10.0/cmake/OpenCVModule.cmake;1375;ocv_add_test_from_target;/workspace/opencv/opencv-4.10.0/cmake/OpenCVModule.cmake;1133;ocv_add_accuracy_tests;/workspace/opencv/opencv-4.10.0/modules/video/CMakeLists.txt;2;ocv_define_module;/workspace/opencv/opencv-4.10.0/modules/video/CMakeLists.txt;0;")
add_test(opencv_perf_video "/workspace/build/bin/opencv_perf_video" "--gtest_output=xml:opencv_perf_video.xml")
set_tests_properties(opencv_perf_video PROPERTIES  LABELS "Main;opencv_video;Performance" WORKING_DIRECTORY "/workspace/build/test-reports/performance" _BACKTRACE_TRIPLES "/workspace/opencv/opencv-4.10.0/cmake/OpenCVUtils.cmake;1795;add_test;/workspace/opencv/opencv-4.10.0/cmake/OpenCVModule.cmake;1274;ocv_add_test_from_target;/workspace/opencv/opencv-4.10.0/cmake/OpenCVModule.cmake;1134;ocv_add_perf_tests;/workspace/opencv/opencv-4.10.0/modules/video/CMakeLists.txt;2;ocv_define_module;/workspace/opencv/opencv-4.10.0/modules/video/CMakeLists.txt;0;")
add_test(opencv_sanity_video "/workspace/build/bin/opencv_perf_video" "--gtest_output=xml:opencv_perf_video.xml" "--perf_min_samples=1" "--perf_force_samples=1" "--perf_verify_sanity")
set_tests_properties(opencv_sanity_video PROPERTIES  LABELS "Main;opencv_video;Sanity" WORKING_DIRECTORY "/workspace/build/test-reports/sanity" _BACKTRACE_TRIPLES "/workspace/opencv/opencv-4.10.0/cmake/OpenCVUtils.cmake;1795;add_test;/workspace/opencv/opencv-4.10.0/cmake/OpenCVModule.cmake;1275;ocv_add_test_from_target;/workspace/opencv/opencv-4.10.0/cmake/OpenCVModule.cmake;1134;ocv_add_perf_tests;/workspace/opencv/opencv-4.10.0/modules/video/CMakeLists.txt;2;ocv_define_module;/workspace/opencv/opencv-4.10.0/modules/video/CMakeLists.txt;0;")
