import tensorrt as trt

# 엔진 파일 경로
engine_file_path = '/workspace/sources/apps/sample_apps/deepstream-3d-action-recognition/falldown_3d_config/c3d_fp16_sport1M_falldown_rgb.engine'

# TensorRT 로그 수준 설정
TRT_LOGGER = trt.Logger(trt.Logger.WARNING)

# 엔진 로딩 (deserialize_cuda_engine_from_file 사용)
with open(engine_file_path, 'rb') as f:
    engine_data = f.read()

runtime = trt.Runtime(TRT_LOGGER)
engine = runtime.deserialize_cuda_engine(engine_data)

if engine is None:
    print("Failed to load the engine.")
else:
    # 엔진의 입력 텐서 차원 확인
    for binding in engine:
        if engine.binding_is_input(binding):
            print(f"Input binding: {binding}")
            shape = engine.get_binding_shape(binding)
            print(f"Shape of input tensor {binding}: {shape}")

