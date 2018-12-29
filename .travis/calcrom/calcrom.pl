#!/usr/bin/perl

(@ARGV == 1)
    or die "ERROR: no map file specified.\n";
open(my $file, $ARGV[0])
    or die "ERROR: could not open file '$ARGV[0]'.\n";

my $src = 0;
my $asm = 0;
my $undocumented = 0;
while (my $line = <$file>)
{
    if ($line =~ /^ \.(\w+)\s+0x[0-9a-f]+\s+(0x[0-9a-f]+) (\w+)\/.+\.o/)
    {
        my $section = $1;
        my $size = hex($2);
        my $dir = $3;

        if ($section =~ /text/)
        {
            if ($dir eq 'src')
            {
                $src += $size;
            }
            elsif ($dir eq 'asm')
            {
                $asm += $size;
            }
        }
    }
    if($line =~ /^\s+0x([0-9A-f]+)\s+[A-z_]+([0-9A-f]+)/) {
        my $thing1 = sprintf("%08X", hex($1));
        my $thing2 = sprintf("%08X", hex($2));
        if($thing1 eq $thing2) {
            $undocumented += 1;
        }
    }
}

my $total = $src + $asm;
my $srcPct = sprintf("%.4f", 100 * $src / $total);
my $asmPct = sprintf("%.4f", 100 * $asm / $total);
print "$total total bytes of code\n";
print "$src bytes of code in src ($srcPct%)\n";
print "$asm bytes of code in asm ($asmPct%)\n";
print "$undocumented global names undocumented\n";
