# -*- coding: utf-8 -*-
import os
import argparse
import sys

# Git 병합 충돌 마커
CONFLICT_MARKERS = [
    b'<<<<<<< HEAD',
    b'======',
    b'>>>>>>>'
]

# 스캔에서 제외할 디렉터리 목록
# 빌드 아티팩트, 캐시, 버전 관리 시스템 내부 폴더 등
IGNORE_DIRS = {
    '.git',
    'Binaries',
    'Intermediate',
    'DerivedDataCache',
    'Saved',
    '.vs',
    '.idea',
    '__pycache__',
    'node_modules',
}

def find_lfs_conflicts(start_dir):
    """
    지정된 디렉터리부터 시작하여 Git LFS 병합 충돌이 의심되는 파일을 찾습니다.

    :param start_dir: 스캔을 시작할 루트 디렉터리
    :return: 충돌이 감지된 파일 경로의 리스트
    """
    conflicted_files = []
    
    # 디렉터리가 존재하는지 확인
    if not os.path.isdir(start_dir):
        print(f"오류: 디렉터리를 찾을 수 없습니다: {start_dir}", file=sys.stderr)
        return conflicted_files

    # 현재 실행 중인 스크립트 파일의 경로
    current_script_path = os.path.abspath(__file__)

    # os.walk를 사용하여 디렉터리 트리 순회
    for root, dirs, files in os.walk(start_dir):
        # 제외할 디렉터리 필터링 (효율성을 위해)
        dirs[:] = [d for d in dirs if d not in IGNORE_DIRS]

        for file in files:
            file_path = os.path.join(root, file)
            
            # 현재 실행 중인 스크립트 파일은 검사에서 제외
            if os.path.abspath(file_path) == current_script_path:
                continue
            
            try:
                # 파일을 바이너리 모드로 열어 인코딩 문제 방지
                with open(file_path, 'rb') as f:
                    content = f.read()
                    
                    # 모든 충돌 마커가 파일 내용에 존재하는지 확인
                    if all(marker in content for marker in CONFLICT_MARKERS):
                        conflicted_files.append(file_path)

            except IOError as e:
                # 파일 접근 중 오류 발생 시 (예: 권한 문제)
                print(f"경고: {file_path} 파일을 읽을 수 없습니다: {e}", file=sys.stderr)

    return conflicted_files

def main():
    """
    스크립트의 메인 실행 함수.
    """
    parser = argparse.ArgumentParser(
        description="프로젝트 내에서 Git LFS 포인터 병합 충돌을 감지합니다.",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        '--dir',
        type=str,
        default='.',
        help='검사를 시작할 디렉터리 경로입니다. (기본값: 현재 디렉터리)'
    )
    args = parser.parse_args()

    start_directory = os.path.abspath(args.dir)
    
    print(f"'{start_directory}' 디렉터리에서 Git LFS 병합 충돌 검사를 시작합니다...")
    
    conflicted = find_lfs_conflicts(start_directory)

    if conflicted:
        print("\n[오류] 아래 파일에서 Git LFS 병합 충돌이 감지되었습니다:", file=sys.stderr)
        for file in conflicted:
            # 사용자가 클릭하여 파일로 바로 이동할 수 있도록 표준 형식으로 출력
            print(f"  - {file}", file=sys.stderr)
        
        print("\nLFS 포인터가 깨졌을 가능성이 높습니다. 파일을 열어 충돌을 해결해주세요.", file=sys.stderr)
        sys.exit(1) # 오류 코드로 종료 (CI/CD 환경에서 활용)
    else:
        print("\n[성공] Git LFS 병합 충돌이 감지되지 않았습니다.")
        sys.exit(0)

if __name__ == "__main__":
    main()
