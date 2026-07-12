#!/usr/bin/env perl

use strict;
use warnings;

use Cwd qw(abs_path);
use File::Basename qw(basename);
use File::Path qw(make_path);
use FindBin;
use Text::ParseWords qw(shellwords);

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

my $source_argument = shift
	or die "usage: $0 <source.c|source.pas>\n";

die "unexpected argument: @ARGV\n" if @ARGV;

my $source = abs_path($source_argument)
	or die "source not found: $source_argument\n";

die "unsupported source file: $source\n"
	unless $source =~ /\.(?:c|pas)\z/i;

my $emulator_dir = abs_path($FindBin::Bin);
my $compiler_dir = abs_path("$emulator_dir/../compiler");
my $lib_dir = abs_path("$emulator_dir/include");
my $lib_src_dir = abs_path("$emulator_dir/src");

my $out_dir = "$emulator_dir/out";
my $pcc = "$compiler_dir/out/pcc";
my $pocketgame_header = "$lib_dir/pocketgame.h";
my $pocketgame_runtime = "$lib_src_dir/pocketgame.c";
my $pocketgame_main = "$lib_src_dir/pocketgame_main.c";
my $pocketgame_library = "$out_dir/libpocketgame.a";

make_path($out_dir);

die "missing file: $pocketgame_header\n"
	unless -f $pocketgame_header;

die "missing file: $pocketgame_runtime\n"
	unless -f $pocketgame_runtime;

die "missing file: $pocketgame_main\n"
	unless -f $pocketgame_main;

print "Building PocketGame emulator...\n";
run_command("make", "-C", $emulator_dir, "all");

my $name = basename($source);
$name =~ s/\.(?:c|pas)\z//i;

my $pas_source;

if ($source =~ /\.c\z/i) {
	print "Building pcc...\n";
	run_command("make", "-C", $compiler_dir, "all");

	die "pcc not found: $pcc\n"
		unless -x $pcc;

	$pas_source = "$out_dir/$name.pas";

	print "Compiling $source...\n";
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

my $target = "$out_dir/$name";
my @sdl_cflags = command_output("sdl2-config", "--cflags");
my @sdl_libs = command_output("sdl2-config", "--libs");

print "Building $target...\n";
run_command(
	"gcc",
	"-Wall",
	"-Wextra",
	"-Werror",
	"-Wno-unused-label",
	"-std=c11",
	"-I$emulator_dir/include",
	"-I$lib_dir",
	@sdl_cflags,
	"-include",
	"pocketgame.h",
	"-x",
	"c",
	$pas_source,
	"-x",
	"none",
	$pocketgame_main,
	$pocketgame_runtime,
	$pocketgame_library,
	@sdl_libs,
	"-o",
	$target,
);

print "Running $target...\n";
run_command($target);
