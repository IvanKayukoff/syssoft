#!/usr/bin/perl -T

use strict;
use warnings qw(FATAL all);

use Getopt::Long;

my $l = 0;
my $s = 0;

GetOptions("s"  => \$s, "l"  => \$l) or die("Error in command line arguments\n");
die "usage: cmp [-l] [-s] file1 file2\n" if scalar(@ARGV) != 2;

my $file1 = $ARGV[0];
my $file2 = $ARGV[1];

my $f1;
my $f2;

if ($file1 eq "-") {
  $f1=\*STDIN;
}
else {
  open($f1, '<', $file1) or die "cmp: cannot open '$file1'";
}

if ($file2 eq "-") {
  $f2=\*STDIN;
}
else {
  open($f2, '<', $file2) or die "cmp: cannot open '$file2'";
}

my @mfile1;
my @mfile2;
my $i1 = 0;
my $i2 = 0;

while (my $row = <$f1>) {
  $mfile1[ $i1 ]=$row;
  $i1++;
}

while (my $row = <$f2>) {
  $mfile2[ $i2 ] = $row;
  $i2++;
}

if (($i1 == 0) && ($i2 != 0)) {
  print "cmp: EOF on $file1\n";
  exit 1;
}

if (($i1 != 0) && ($i2 == 0)) {
  print "cmp: EOF on $file2\n";
  exit 1;
}

my $strings;

if ($i1 > $i2) {
  $strings = $i2;
} else {
  $strings = $i1;
}

my $chars = 0;
my $flag = 0;
my $i = 0;

loop: {
  for (; $i < $strings; $i++) { 
    my @f1_temp = split(//, $mfile1[$i]); 
    my @f2_temp = split(//, $mfile2[$i]); 
    my $t_len1 = scalar(@f1_temp);

    if ($t_len1 > scalar(@f2_temp)) {
      $t_len1 = scalar(@f2_temp);
    }

    for (my $k = 0; $k < $t_len1; $k++) {
      $chars++; 
      if ($f1_temp[$k] ne $f2_temp[$k]) {
        if (($l == 1) && ($s != 1)) {
          printf("%d %o %o \n", $chars, ord($f1_temp[$k]), ord($f2_temp[$k]));
        }
        $flag=1;
        if ($l == 0) {
          last loop;
        }
      }
    }
  }
}

if (($s == 1) || ($l == 1)) {
  if ((($flag == 0) && ($i1 != $i2)) || ($flag == 1)) {
    exit 1;
  }
} else {
  if (($flag == 0) && ($i1 > $i2)) {
    print "cmp: EOF on $file2\n";
    exit 1;
  } elsif (($flag == 0) && ($i1 < $i2)) {
    print "cmp: EOF on $file1\n";
    exit 1;
  }
  if ($flag == 1) { 
    $i++;
    print "$file1 $file2 differ: char $chars, line $i\n";
    exit 1;
  }
}

close($f1);
close($f2);
