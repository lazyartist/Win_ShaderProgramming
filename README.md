# Win_ShaderProgramming

# D3D
## 구조체
- 모델
    - LPD3DXMESH
- 쉐이더
    - LPD3DXEFFECT
- 벡터
    - D3DXVECTOR3
    - D3DXVECTOR4
- 텍스쳐
    - LPDIRECT3DTEXTURE9
- 매트릭스
    - D3DXMATRIX
## 함수
- 뷰 매트릭스 만들기
    - D3DXMatrixLookAtLH
- 프로젝션 매트릭스 만들기
    - D3DXMatrixPerspectiveFovLH
## 주의사항
- 쉐이더 텍스처 이름 끝에 "_Tex"를 붙여야함
## 좌표계
- D3D도 렌더몽키와 마찬가지로 왼손 좌표계
- Forward Vector는 z축

# Chapters
## 5. Specular Mapping
- 보간기를 거치기전에 거듭제곱을 한 값과 거친 후의 거듭제곱 값은 차이가 크므로 반사광을 PS에서 구하고 거듭제곱한다. specular 값을 VS에서 구하고 PS에서는 거듭제곱만 해도 결과는 동일한거 같다.

## 6. Toon Shader

## 7. Normal Mapping
- NormalMap에 정의하는 Normal의 공간을 픽셀 공간 대신 물체의 공간으로 하면 펙셀쉐이더에서 공간변환할 필요가 없을거 같은 생각이 든다.
- 하지만 이는 잘 못된 생각
- 접선 공간은 폴리곤이라는 면에서의 공간이다.
- 서로 다른 폴리곤의 표면에서 나온 노멀이 같다면 물체공간에서 같은 방향인 평면이다.
- 하지만 접선공간(Tangent Space)에서 왼쪽 벡터는 모든 접선공간에서 왼쪽이다.
- 하지만 물체공간에서는 평면의 방향에 따라 달라질 수 있다.
- 평면의 방향에 따라 다른 달라져야 하는 법선을 같은 공간이 물체 공간에서 저장할 수는 없다.
- 만약 저장하면 표면에서의 굴곡이 아닌 물체 공간에서의 굴곡. 즉, 실제 평면의 방향인 노멀로 해석된다.
- 간단하게
- 각각의 폴리곤에서의 굴곡을 나타내므로 폴리곤의 지역공간에서 정의되어야 하고 이를 사용할 때는 월드공간으로 변환해줘야 한다.
- 이 폴리곤의 지역공간을 Tangent Space라고 한다.
이를 정의하기 위해 Normal, Tangent, Binormal이 필요하다.
Normal: Vertex의 Normal
Tangent: Vertex의 접선(평면의 X)
Binormal: Normal x Tangent 외적
이 세개의 백터를 축으로하는 공간을 정의할 수 있으며 이를 접선공간(Tangent Space)라고 한다.


접선 공간의 노멀을 물체 공간에서의 노멀로 저장해도 결과는 같다.
하지만 이렇게 되면 메쉬를 수정했을 때 접선공간의 노멀도 함께 수정해야한다.
이 노멀은 물체공간에 있기 때문이다. 
표면공간에 있으면 메쉬를 수정해도(크기 변화 없이 방향만 수정한 경우) 법선맵을 다시 수정할 필요가 없지만
이미 물체 공간에 있음을 가정하고 저장된 노멀이기 때문에 폴리곤 방향이 수정될 때마다 함께 수정되어야 한다.
이는 마치 지구가 공전할 때마다. 지구상의 모든 물체의 위치값을 갱신시키는 것과 같다.
지구 표면에 위치한 물체는 지구가 공전해도 지구표면에서의 좌표는 변경되지 않는다.
이와 마찬가지로 폴리곤 단위의 면 위에서의 노멀은 표면공간에서 상대적으로 정의 되어야 한다.


정리 필요

# 월드, 지역, 물체 공간과 공간 변환
- 메트릭스는 도구일뿐 매트릭스가 직접적으로 공간변환과 이동, 회전, 크기 행렬을 의미하지 않는다.
- 공간변환 == 좌표계 변환, 공견변환 != 위치 변환
    - 정점의 위치는 변하지 않는다.
    좌표계만 변했으므로 정점의 위치를 나타내는 새로운 좌표가 생긴것뿐이다.
- 공간변환 == 다른 좌표계에서 좌표 구하기
- 탄젠트 공간은 물체 좌표계에서 틀어진 좌표계이므로 틀어진 값을 상쇄시키는 역행렬을 탄젠트 공간에 있는 벡터에 적용해 상쇄시키면 월드공간(탄젠트 공간의 표준좌표계 또는 물체공간)에서의 좌표를 구할 수 있다.
- 빛과의 각도를 구하려면 탄젠트공간->물체공간->월드공간으로 변환해야한다.
    - 결국 부모좌표계에서의 벡터를 구하는 것.
탄젠트 좌표계처럼 틀어진 좌표계는 표준(기저) 좌표계 대비 변경된 값을 갖는다.

- 탄젠트 좌표계 방향
    - X: 오른쪽 +
    - Y: 아래쪽 +
    - Z: (스크린)바깥쪽 +
    - 왼손으로 X, Y 순서로 감싸쥐었을 때 엄지가 바깥쪽을 향하므로 왼손 좌표계

- 행렬과 벡터의 곱 기준
    - 열기준 변환행렬
        - 가로 배치 행렬 x 세로 배치 벡터
        - mul(Matrix, Vector)
        - 열 벡터와 곱하므로 행렬이 첫번째 항이 되고 행렬의 요소는 가로 배치, 벡터는 두번째 항.
            | Tx Ty Tz |   | Vx |
            | Bx By Bz | x | Vy |
            | Nx Ny Nz |   | Vz | 
    - 행기준 변환행렬
        - 가로 배치 벡터 x 세로 배치 행렬
        - mul(Vector, Matrix)
        - 행 벡터와 곱하므로 벡터가 첫번째 항, 행렬은 두번째 항이고 행력의 요소는 세로 배치.
            | Vx Vy Vz |   | Tx Bx Nx |
                         x | Ty By Ny |
                           | Tz Bz Nz |
    - 벡터와 행렬의 요소를 곱할 때 같은 요소끼리(x->x, y->y, z->z) 곱한다.
    - 행렬 곱은 가로->세로 방향으로 곱한다.
    - 따라서 첫번째 항의 가로 방향과 두 번째 항의 세로 방향에 원소를 배치하면 된다.
    - 행, 열 기준은 벡터가 기준이다.

# 렌더몽키 float3x3 행렬
- 행 기준
