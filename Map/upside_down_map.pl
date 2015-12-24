@ARGV = ('upside_down_map.txt');
open $fh, '>upside_down_mapped.txt' or die 'Unable to open file for writing.';

$normal = 'char normal[] = {';
$mirrored = 'std::string mirrored[] = {';

while (<>) {
    @F = split ' ', $_;
    $normal .= " '$F[0]',";
    $mirrored .= " \"$F[1]\",";
}

$normal .= ' };';
$mirrored .= ' };';

print $fh $normal . "\n\n";
print $fh $mirrored . "\n\n";

close $fh;