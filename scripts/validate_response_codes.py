# -*- coding: utf-8 -*-
import json
import re
import sys
from pathlib import Path

def parse_cpp_for_codes(file_path: Path) -> set:
    """C++ 파일에서 응답 코드를 파싱합니다."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except IOError as e:
        print(f"Error: Could not read C++ file: {e}", file=sys.stderr)
        return set()

    # 마커 사이의 내용만 추출
    start_marker = "// @MVE_RESPONSE_CODE_CHECKER:START"
    end_marker = "// @MVE_RESPONSE_CODE_CHECKER:END"
    
    try:
        start_index = content.index(start_marker)
        end_index = content.index(end_marker, start_index)
        search_area = content[start_index:end_index]
    except ValueError:
        print(f"Error: Could not find start/end markers in {file_path}", file=sys.stderr)
        print("Please add '// @MVE_RESPONSE_CODE_CHECKER:START' and '// @MVE_RESPONSE_CODE_CHECKER:END' markers.", file=sys.stderr)
        return set()

    # 정규식을 사용하여 코드 추출
    # 예: ResponseCodeToTextMap.Emplace(TEXT("SOME_CODE"), ...);
    regex = r'ResponseCodeToTextMap\.Emplace\(TEXT\("([^\"]+)"\),' # Corrected regex escaping
    found_codes = re.findall(regex, search_area)
    
    return set(found_codes)

def parse_openapi_spec_for_codes(file_path: Path) -> set:
    """OpenAPI spec 파일에서 응답 코드를 파싱합니다."""
    codes = set()
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            spec = json.load(f)
    except (IOError, json.JSONDecodeError) as e:
        print(f"Warning: Could not read or parse OpenAPI spec {file_path}: {e}", file=sys.stderr)
        return codes

    # Extract codes from 'paths'
    if "paths" in spec:
        for path_item in spec["paths"].values():
            for operation in path_item.values():
                if "responses" in operation:
                    for response in operation["responses"].values():
                        content = response.get("content", {}).get("application/json", {})
                        schema = content.get("schema", {})
                        
                        # Handle direct 'code' property
                        if "properties" in schema and "code" in schema["properties"]:
                            code_example = schema["properties"]["code"].get("example")
                            if code_example:
                                # Some examples might have multiple codes separated by "또는"
                                for code in code_example.split(" 또는 "):
                                    codes.add(code.strip())
                        
                        # Handle $ref to ErrorResponse or SuccessResponse, where code is defined
                        if "$ref" in schema:
                            ref_name = schema["$ref"].split('/')[-1]
                            if ref_name in spec.get("components", {}).get("schemas", {}):
                                ref_schema = spec["components"]["schemas"][ref_name]
                                if "properties" in ref_schema and "code" in ref_schema["properties"]:
                                    code_example = ref_schema["properties"]["code"].get("example")
                                    if code_example:
                                        for code in code_example.split(" 또는 "):
                                            codes.add(code.strip())
    return codes

def parse_json_for_codes(file_paths: list[Path]) -> set:
    """JSON 파일들에서 응답 코드를 파싱합니다."""
    all_codes = set()
    ignored_codes = {"VALUE", "..."} # 분석 스크립트에서 사용되는 메타 코드

    for file_path in file_paths:
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                data = json.load(f)
        except (IOError, json.JSONDecodeError) as e:
            print(f"Warning: Could not read or parse JSON file {file_path}: {e}", file=sys.stderr)
            continue
            
        # Check if it's an OpenAPI spec (has 'openapi' field at root)
        if "openapi" in data and "paths" in data:
            all_codes.update(parse_openapi_spec_for_codes(file_path))
        # Otherwise, assume it's a 'statistics' type JSON
        elif "statistics" in data and isinstance(data["statistics"], list):
            for item in data["statistics"]:
                if "code" in item and item["code"] not in ignored_codes:
                    all_codes.add(item["code"])

    return all_codes

def main():
    """메인 실행 함수"""
    has_errors = False
    project_root = Path(__file__).parent.parent

    # 파일 경로 설정
    cpp_file = project_root / "Source" / "MVE" / "API" / "Private" / "MVE_GIS_API.cpp"
    json_files = [
        project_root / "ApiSpecs" / "login-server-api-spec.json",
        project_root / "ApiSpecs" / "login-server-response-codes.json",
        project_root / "ApiSpecs" / "resource-server-response-codes.json",
    ]

    print("1. Parsing C++ file for implemented response codes...")
    cpp_codes = parse_cpp_for_codes(cpp_file)
    if not cpp_codes:
        sys.exit(1)
    print(f"   Found {len(cpp_codes)} codes in C++.")

    print("\n2. Parsing JSON spec files for defined response codes...")
    json_codes = parse_json_for_codes(json_files)
    if not json_codes:
        sys.exit(1)
    print(f"   Found {len(json_codes)} unique codes in JSON specs.")
    
    # 비교
    missing_in_cpp = json_codes - cpp_codes
    deprecated_in_cpp = cpp_codes - json_codes

    print("\n" + "="*80)
    print("API Response Code Validation Report")
    print("="*80)

    if not missing_in_cpp and not deprecated_in_cpp:
        print("\n✅ [SUCCESS] All response codes are synchronized between JSON specs and C++ code.")
    else:
        if missing_in_cpp:
            has_errors = True
            print("\n❌ [ERROR] Codes defined in API specs but MISSING in C++:")
            for code in sorted(list(missing_in_cpp)):
                print(f"  - {code}")
            print("\n  --&gt; Please add these codes to 'MVE_GIS_API.cpp'.")

        if deprecated_in_cpp:
            # 심각한 오류는 아니므로 경고로 처리
            print("\n⚠️ [WARNING] Codes in C++ but no longer in API specs (deprecated?):")
            for code in sorted(list(deprecated_in_cpp)):
                print(f"  - {code}")
            print("\n  --&gt; Consider removing these codes from 'MVE_GIS_API.cpp'.")
            
    print("\n" + "="*80)

    if has_errors:
        print("\nValidation finished with errors.")
        sys.exit(1)
    else:
        print("\nValidation finished successfully.")
        sys.exit(0)

if __name__ == "__main__":
    main()
