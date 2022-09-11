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

