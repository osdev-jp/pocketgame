#!/usr/bin/env perl

use strict;
use warnings;

use Cwd qw(abs_path);
use File::Basename qw(basename);
use File::Path qw(make_path);
use FindBin;
use Getopt::Long qw(GetOptions);
use Text::ParseWords qw(shellwords);

sub print_usage {
	my ($handle) = @_;

	print {$handle} "usage: $0 [--arch <architecture>] <source.c|source.pas>\n";
	print {$handle} "\n";
	print {$handle} "options:\n";
	print {$handle} "  -a, --arch <architecture>  Target architecture\n";
	print {$handle} "                             Supported: x86_64, pg_rv32\n";
	print {$handle} "  -h, --help                 Show this help\n";
}

sub run_command {
	my (@command) = @_;

	system(@command);

	if ($? == -1) {
		die "failed to execute $command[0]: $!\n";
	}

	if ($? & 127) {
		die "$command[0] terminated by signal " . ($? & 127) . "\n";
	}

	my $status = $? >> 8;

	if ($status != 0) {
		exit $status;
	}
}

sub command_output {
	my (@command) = @_;

	open my $handle, "-|", @command
		or die "failed to execute $command[0]: $!\n";

	local $/;
	my $output = <$handle> // "";

	close $handle
		or die "$command[0] failed\n";

	return shellwords($output);
}

my $architecture = "x86_64";
my $show_help = 0;

GetOptions(
	"arch|a=s" => \$architecture,
	"help|h" => \$show_help,
) or do {
	print_usage(*STDERR);
	exit 1;
};

if ($show_help) {
	print_usage(*STDOUT);
	exit 0;
}

die "unsupported architecture: $architecture\n"
	unless $architecture eq "x86_64"
		|| $architecture eq "pg_rv32";

my $source_argument = shift
	or do {
		print_usage(*STDERR);
		exit 1;
	};

die "unexpected argument: @ARGV\n"
	if @ARGV;

my $source = abs_path($source_argument)
	or die "source not found: $source_argument\n";

die "unsupported source file: $source\n"
	unless $source =~ /\.(?:c|pas)\z/i;

my $emulator_dir = abs_path($FindBin::Bin);
my $compiler_dir = abs_path("$emulator_dir/../compiler");
my $include_dir = abs_path("$emulator_dir/include");
my $src_dir = abs_path("$emulator_dir/src");

my $out_dir = "$emulator_dir/out/$architecture";
my $pcc = "$compiler_dir/out/pcc";
my $pocketgame_header = "$include_dir/pocketgame.h";
my $pocketgame_main = "$src_dir/pocketgame_main.c";
my $pocketgame_library =
	"$emulator_dir/target/$architecture/libpocketgame.a";

my $cc;
my $ar;
my @architecture_cflags;
my @architecture_ldflags;
my @platform_cflags;
my @platform_libs;

if ($architecture eq "x86_64") {
	$cc = $ENV{"X86_64_CC"} // "gcc";
	$ar = $ENV{"X86_64_AR"} // "ar";

	@architecture_cflags = shellwords(
		$ENV{"X86_64_CFLAGS"} // ""
	);

	@architecture_ldflags = shellwords(
		$ENV{"X86_64_LDFLAGS"} // ""
	);

	@platform_cflags = command_output(
		"sdl2-config",
		"--cflags",
	);

	@platform_libs = command_output(
		"sdl2-config",
		"--libs",
	);
} else {
	$cc = $ENV{"PG_RV32_CC"} // "riscv32-unknown-elf-gcc";
	$ar = $ENV{"PG_RV32_AR"} // "riscv32-unknown-elf-ar";

	@architecture_cflags = shellwords(
		$ENV{"PG_RV32_CFLAGS"} // ""
	);

	@architecture_ldflags = shellwords(
		$ENV{"PG_RV32_LDFLAGS"} // ""
	);

	@platform_cflags = ();
	@platform_libs = ();
}

make_path($out_dir);

die "missing file: $pocketgame_header\n"
	unless -f $pocketgame_header;

die "missing file: $pocketgame_main\n"
	unless -f $pocketgame_main;

print "Building PocketGame library for $architecture...\n";
run_command(
	"make",
	"-C",
	$emulator_dir,
	"ARCH=$architecture",
	"CC=$cc",
	"AR=$ar",
	"all",
);

die "missing library: $pocketgame_library\n"
	unless -f $pocketgame_library;

my $name = basename($source);
$name =~ s/\.(?:c|pas)\z//i;

my $pas_source;

if ($source =~ /\.c\z/i) {
	print "Building pcc...\n";
	run_command(
		"make",
		"-C",
		$compiler_dir,
		"all",
	);

	die "pcc not found: $pcc\n"
		unless -x $pcc;

	$pas_source = "$out_dir/$name.pas";

	print "Compiling $source with pcc...\n";
	run_command(
		$pcc,
		"-S",
		$source,
		"-o",
		$pas_source,
	);
} else {
	$pas_source = $source;
}

my $target;

if ($architecture eq "x86_64") {
	$target = "$out_dir/$name";
} else {
	$target = "$out_dir/$name.elf";
}

print "Building $target for $architecture...\n";
run_command(
	$cc,
	"-Wall",
	"-Wextra",
	"-Werror",
	"-Wno-unused-label",
	"-std=c11",
	@architecture_cflags,
	@platform_cflags,
	"-I$include_dir",
	"-include",
	"pocketgame.h",
	"-x",
	"c",
	$pas_source,
	"-x",
	"none",
	$pocketgame_main,
	$pocketgame_library,
	@architecture_ldflags,
	@platform_libs,
	"-o",
	$target,
);

if ($architecture eq "x86_64") {
	print "Running $target...\n";
	run_command($target);
} else {
	print "Built pg_rv32 executable: $target\n";
}
