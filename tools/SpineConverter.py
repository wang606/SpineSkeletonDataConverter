import argparse
import subprocess
import sys
from pathlib import Path


def format_command(command: list[str]) -> str:
	return subprocess.list2cmdline(command)


def locate_executable(name: str) -> Path:
	script_dir = Path(__file__).resolve().parent
	direct_candidate = script_dir / name
	if direct_candidate.exists():
		return direct_candidate

	for candidate in script_dir.rglob(name):
		if candidate.is_file():
			return candidate

	raise FileNotFoundError(
		f"Could not locate {name} under {script_dir}. Ensure the executable is placed alongside this script."
	)


def determine_output_suffix(source_suffix: str, format_option: str) -> str:
	source_suffix = source_suffix.lower()
	if format_option == "same":
		return source_suffix
	if format_option == "json":
		return ".json"
	if format_option == "skel":
		return ".skel"
	if format_option == "other":
		if source_suffix == ".json":
			return ".skel"
		if source_suffix == ".skel":
			return ".json"
		raise ValueError("--format other only applies to .json or .skel files")
	raise ValueError(f"Unsupported format option: {format_option}")


def build_converter_command(executable: Path, input_path: Path, output_path: Path, version: str | None, remove_curve: bool) -> list[str]:
	command = [str(executable), str(input_path), str(output_path)]
	if version:
		command.extend(["-v", version])
	if remove_curve:
		command.append("--remove-curve")
	return command


def build_atlas_command(executable: Path, input_path: Path, output_dir: Path) -> list[str]:
	return [str(executable), str(input_path), str(output_dir)]


def process_files(args: argparse.Namespace) -> None:
	input_dir = Path(args.input_directory).resolve()
	output_dir = Path(args.output_directory).resolve()

	if not input_dir.exists() or not input_dir.is_dir():
		raise FileNotFoundError(f"Input directory does not exist: {input_dir}")

	output_dir.mkdir(parents=True, exist_ok=True)

	converter_exe = locate_executable("SpineSkeletonDataConverter.exe")
	atlas_exe = locate_executable("SpineAtlasDowngrade.exe")

	for path in input_dir.rglob("*"):
		if not path.is_file():
			continue

		suffix = path.suffix.lower()
		if suffix not in {".json", ".skel", ".atlas"}:
			continue

		relative_path = path.relative_to(input_dir)
		destination_parent = (output_dir / relative_path).parent
		destination_parent.mkdir(parents=True, exist_ok=True)

		if suffix in {".json", ".skel"}:
			output_suffix = determine_output_suffix(suffix, args.format)
			destination_path = output_dir / relative_path.with_suffix(output_suffix)
			command = build_converter_command(
				converter_exe,
				path,
				destination_path,
				args.version,
				args.remove_curve,
			)
			print(f"[converter] {format_command(command)}")
			subprocess.run(command, check=True)
		else:  # .atlas
			command = build_atlas_command(atlas_exe, path, output_dir / relative_path.parent)
			print(f"[atlas] {format_command(command)}")
			subprocess.run(command, check=True)


def parse_arguments(argv: list[str]) -> argparse.Namespace:
	parser = argparse.ArgumentParser(description="Batch convert Spine assets using project tools")
	parser.add_argument("input_directory", help="Root directory containing source files")
	parser.add_argument("output_directory", help="Destination directory for converted files")
	parser.add_argument(
		"-v",
		"--version",
		help="Target Spine version (x.y.z). Defaults to source version.",
	)
	parser.add_argument(
		"--format",
		choices=["same", "json", "skel", "other"],
		default="same",
		help="Output format selection rule for .json/.skel files",
	)
	parser.add_argument(
		"--remove-curve",
		action="store_true",
		help="Strip animation curves instead of converting when crossing 3.x/4.x",
	)
	return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
	if argv is None:
		argv = sys.argv[1:]

	try:
		args = parse_arguments(argv)
		process_files(args)
	except FileNotFoundError as exc:
		print(f"Error: {exc}", file=sys.stderr)
		return 1
	except subprocess.CalledProcessError as exc:
		command_repr = format_command(list(exc.cmd)) if isinstance(exc.cmd, (list, tuple)) else str(exc.cmd)
		print(f"Subprocess failed with exit code {exc.returncode}: {command_repr}", file=sys.stderr)
		return exc.returncode or 1
	except ValueError as exc:
		print(f"Error: {exc}", file=sys.stderr)
		return 1

	return 0


if __name__ == "__main__":
	raise SystemExit(main())
