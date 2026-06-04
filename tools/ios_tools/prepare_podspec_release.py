#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

import argparse
import os
import re
import sys
from pathlib import Path
from urllib.parse import urlparse


ROOT_SPEC_PATTERN = re.compile(r"Pod::Spec\.new\s+do\s+\|([A-Za-z_][A-Za-z0-9_]*)\|")
COMMIT_ID_PATTERN = re.compile(r"^[0-9a-fA-F]{7,40}$")
SHA256_PATTERN = re.compile(r"^[0-9a-fA-F]{64}$")
ANIMAX_RELEASE_SOURCE_PATH_PREFIX = "/lynx-family/animax/releases/download/"
DEFAULT_DEPENDENCY_VERSIONS_PATH = (
    Path(__file__).resolve().parents[2] / "versions.properties"
)
REQUIRED_DEPENDENCY_VERSION_KEYS = (
    "LYNX_VERSION",
    "SKITY_VERSION",
    "TEXTRA_VERSION",
)
RELEASE_PRESERVE_PATHS = (
    "AnimaX/**/*.h",
    "base/trace/native/**/*.h",
)
RELEASE_PUBLIC_HEADERS = ("AnimaX/**/*.h",)
RELEASE_PRIVATE_HEADERS = ("base/trace/native/**/*.h",)
CORE_RELEASE_PRESERVE_PATHS = RELEASE_PRESERVE_PATHS


def ruby_array(values):
    return "[" + ", ".join(f"'{ruby_single_quoted(value)}'" for value in values) + "]"


def ruby_single_quoted(value):
    return value.replace("\\", "\\\\").replace("'", "\\'")


def require_env(name):
    value = os.environ.get(name, "").strip()
    if not value:
        raise RuntimeError(f"Missing required environment variable: {name}")
    return value


def require_commit_id(name):
    commit_id = require_env(name)
    if not COMMIT_ID_PATTERN.fullmatch(commit_id):
        raise RuntimeError(f"{name} must be a git SHA (7-40 hex chars).")
    return commit_id


def require_source_zip_sha256():
    source_zip_sha256 = require_env("ANIMAX_SOURCE_ZIP_SHA256").lower()
    if not SHA256_PATTERN.fullmatch(source_zip_sha256):
        raise RuntimeError("ANIMAX_SOURCE_ZIP_SHA256 must be a SHA-256 hex digest.")
    return source_zip_sha256


def validate_source_zip_url(source_zip_url):
    parsed_url = urlparse(source_zip_url)
    if (
        parsed_url.scheme != "https"
        or parsed_url.netloc != "github.com"
        or not parsed_url.path.startswith(ANIMAX_RELEASE_SOURCE_PATH_PREFIX)
        or not parsed_url.path.endswith(".zip")
        or parsed_url.params
        or parsed_url.query
        or parsed_url.fragment
    ):
        raise RuntimeError(
            "ANIMAX_SOURCE_ZIP_URL must be an https GitHub release asset URL under "
            "github.com/lynx-family/animax/releases/download/."
        )
    return source_zip_url


def find_root_spec_variable(content):
    match = ROOT_SPEC_PATTERN.search(content)
    if not match:
        raise RuntimeError("Could not find root Pod::Spec variable in generated podspec.")
    return match.group(1)


def replacement_lines(indent, spec_variable, attribute):
    if attribute == "version":
        pod_version = require_env("POD_VERSION")
        return [
            f"{indent}{spec_variable}.version = '{ruby_single_quoted(pod_version)}'\n",
        ]
    if attribute == "source":
        source_zip_url = os.environ.get("ANIMAX_SOURCE_ZIP_URL", "")
        if source_zip_url:
            source_zip_url = validate_source_zip_url(source_zip_url)
            source_zip_sha256 = require_source_zip_sha256()
            return [
                (
                    f"{indent}{spec_variable}.source = "
                    f"{{ :http => '{ruby_single_quoted(source_zip_url)}', "
                    f":sha256 => '{source_zip_sha256}' }}\n"
                ),
                f"{indent}{spec_variable}.static_framework = true\n",
            ]
        commit_id = require_commit_id("ANIMAX_COMMIT_ID")
        return [
            (
                f"{indent}{spec_variable}.source = "
                f"{{ :git => 'https://github.com/lynx-family/animax.git', "
                f":commit => '{commit_id}' }}\n"
            ),
            f"{indent}{spec_variable}.static_framework = true\n",
        ]
    raise ValueError(f"Unsupported podspec attribute: {attribute}")


def find_assignment_end(lines, start_index, attribute):
    stripped = lines[start_index].split("=", 1)[1].strip()
    if attribute == "version" and stripped == "begin":
        for index in range(start_index + 1, len(lines)):
            if lines[index].strip() == "end":
                return index + 1
        raise RuntimeError("Could not find end of pod version block in generated podspec.")

    if attribute == "source":
        brace_balance = stripped.count("{") - stripped.count("}")
        if brace_balance > 0:
            for index in range(start_index + 1, len(lines)):
                brace_balance += lines[index].count("{") - lines[index].count("}")
                if brace_balance <= 0:
                    return index + 1
            raise RuntimeError("Could not find end of pod source block in generated podspec.")

        if re.search(r"\bdo\b", stripped):
            for index in range(start_index + 1, len(lines)):
                if lines[index].strip() == "end":
                    return index + 1
            raise RuntimeError("Could not find end of pod source block in generated podspec.")

    return start_index + 1


def replace_assignment(content, spec_variable, attribute):
    lines = content.splitlines(keepends=True)
    assignment_pattern = re.compile(rf"^(\s*){re.escape(spec_variable)}\.{attribute}\s*=")

    for index, line in enumerate(lines):
        match = assignment_pattern.match(line)
        if not match:
            continue

        end_index = find_assignment_end(lines, index, attribute)
        lines[index:end_index] = replacement_lines(match.group(1), spec_variable, attribute)
        return "".join(lines)

    raise RuntimeError(f"Could not replace pod {attribute} in generated podspec.")


def ensure_release_header_attributes(content, spec_variable):
    if (
        f"{spec_variable}.preserve_paths" in content
        and "AnimaX/**/*.h" in content
        and "base/trace/native/**/*.h" in content
    ):
        return content

    static_framework_pattern = re.compile(
        rf"^(\s*){re.escape(spec_variable)}\.static_framework\s*=\s*true\s*$",
        re.MULTILINE,
    )
    match = static_framework_pattern.search(content)
    if not match:
        raise RuntimeError("Could not find pod static_framework assignment in generated podspec.")

    indent = match.group(1)
    release_header_lines = [
        f"{indent}{spec_variable}.preserve_paths = {ruby_array(RELEASE_PRESERVE_PATHS)}",
        f"{indent}{spec_variable}.source_files = {ruby_array(RELEASE_PRESERVE_PATHS)}",
        f"{indent}{spec_variable}.public_header_files = {ruby_array(RELEASE_PUBLIC_HEADERS)}",
        f"{indent}{spec_variable}.private_header_files = {ruby_array(RELEASE_PRIVATE_HEADERS)}",
    ]
    insertion = "\n".join(release_header_lines)
    return (
        content[: match.end()]
        + "\n"
        + insertion
        + content[match.end() :]
    )


def find_subspec_block(lines, subspec_name):
    subspec_pattern = re.compile(
        rf"^(\s*)[A-Za-z_][A-Za-z0-9_]*\.subspec\s+"
        rf"['\"]{re.escape(subspec_name)}['\"]\s+do\s+"
        r"\|([A-Za-z_][A-Za-z0-9_]*)\|"
    )

    for start_index, line in enumerate(lines):
        match = subspec_pattern.match(line)
        if not match:
            continue

        depth = 0
        for index in range(start_index, len(lines)):
            stripped = lines[index].strip()
            if re.search(r"\bdo\b", stripped):
                depth += 1
            if stripped == "end":
                depth -= 1
                if depth == 0:
                    return start_index, index + 1, match.group(1), match.group(2)
        raise RuntimeError(f"Could not find end of {subspec_name} subspec block.")

    raise RuntimeError(f"Could not find {subspec_name} subspec block.")


def ensure_core_release_preserve_paths(content):
    lines = content.splitlines(keepends=True)
    start_index, end_index, indent, spec_variable = find_subspec_block(lines, "Core")
    block = "".join(lines[start_index:end_index])
    if (
        f"{spec_variable}.preserve_paths" in block
        and "AnimaX/**/*.h" in block
        and "base/trace/native/**/*.h" in block
    ):
        return content

    insert_index = start_index + 1
    header_mappings_pattern = re.compile(
        rf"^\s*{re.escape(spec_variable)}\.header_mappings_dir\s*="
    )
    for index in range(start_index + 1, end_index):
        if header_mappings_pattern.match(lines[index]):
            insert_index = index + 1
            break

    lines.insert(
        insert_index,
        f"{indent}  {spec_variable}.preserve_paths             = "
        f"{ruby_array(CORE_RELEASE_PRESERVE_PATHS)}\n",
    )
    return "".join(lines)


def pin_dependency(content, dependency, version, required=True):
    dependency_pattern = re.compile(
        rf"^(\s*[A-Za-z_][A-Za-z0-9_]*\.dependency\s+['\"]{re.escape(dependency)}['\"])(?:\s*,\s*['\"][^'\"]+['\"])?(\s*)$",
        re.MULTILINE,
    )
    content, replacement_count = dependency_pattern.subn(
        rf"\g<1>, '{version}'\g<2>",
        content,
    )
    if required and replacement_count == 0:
        raise RuntimeError(f"Could not find {dependency} dependency in generated podspec.")
    return content


def read_dependency_versions(path):
    if not path.is_file():
        raise RuntimeError(f"Missing dependency versions file: {path}")

    dependency_versions = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if "=" not in line:
            raise RuntimeError(f"Invalid dependency version line in {path}: {line}")
        key, value = line.split("=", 1)
        dependency_versions[key.strip()] = value.strip()

    missing_keys = [
        key for key in REQUIRED_DEPENDENCY_VERSION_KEYS if not dependency_versions.get(key)
    ]
    if missing_keys:
        raise RuntimeError(
            f"Missing dependency versions in {path}: {', '.join(missing_keys)}"
        )
    return dependency_versions


def remove_dependency(content, dependency):
    dependency_pattern = re.compile(
        rf"^\s*[A-Za-z_][A-Za-z0-9_]*\.dependency\s+['\"]{re.escape(dependency)}['\"].*$\n?",
        re.MULTILINE,
    )
    return dependency_pattern.sub("", content)


def remove_subspec(content, subspec):
    lines = content.splitlines(keepends=True)
    start_pattern = re.compile(
        rf"^\s*[A-Za-z_][A-Za-z0-9_]*\.subspec\s+['\"]{re.escape(subspec)}['\"]\s+do\b"
    )

    for start_index, line in enumerate(lines):
        if not start_pattern.search(line):
            continue

        depth = 0
        for index in range(start_index, len(lines)):
            stripped = lines[index].strip()
            if re.search(r"\bdo\b", stripped):
                depth += 1
            if stripped == "end":
                depth -= 1
                if depth == 0:
                    del lines[start_index : index + 1]
                    return "".join(lines)
        raise RuntimeError(f"Could not find end of {subspec} subspec block.")

    return content


def remove_disabled_trace(content):
    if os.environ.get("ANIMAX_KEEP_TRACE_SUBSPEC") == "1":
        return content
    content = remove_dependency(content, "AnimaX/Trace")
    content = remove_subspec(content, "Trace")
    return content


def prepare_podspec(path, dependency_versions):
    content = path.read_text(encoding="utf-8")
    spec_variable = find_root_spec_variable(content)
    content = replace_assignment(content, spec_variable, "version")
    content = replace_assignment(content, spec_variable, "source")
    content = ensure_release_header_attributes(content, spec_variable)
    content = ensure_core_release_preserve_paths(content)
    content = pin_dependency(content, "skity", dependency_versions["SKITY_VERSION"])
    content = pin_dependency(content, "LynxTextra", dependency_versions["TEXTRA_VERSION"])
    content = pin_dependency(content, "LynxBase/Framework", dependency_versions["LYNX_VERSION"])
    content = pin_dependency(
        content,
        "LynxServiceAPI/Core",
        dependency_versions["LYNX_VERSION"],
        required=False,
    )
    content = remove_disabled_trace(content)
    path.write_text(content, encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description="Prepare generated AnimaX.podspec for CocoaPods trunk release.")
    parser.add_argument("--podspec", default="AnimaX.podspec")
    parser.add_argument(
        "--dependency-versions",
        default=str(DEFAULT_DEPENDENCY_VERSIONS_PATH),
        help="Path to versions.properties.",
    )
    args = parser.parse_args()

    podspec_path = Path(args.podspec)
    if not podspec_path.is_file():
        raise RuntimeError(f"Missing podspec: {podspec_path}")
    dependency_versions = read_dependency_versions(Path(args.dependency_versions))
    prepare_podspec(podspec_path, dependency_versions)
    return 0


if __name__ == "__main__":
    sys.exit(main())
