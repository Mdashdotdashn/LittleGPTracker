@files = split /\s+/, $ARGV[0];
@searchdirs = split /\s+/, $ARGV[1];
map { s/\.o$// } @files;

open IN, "<$ARGV[2]" || die;
for (<IN>) { print };

for $dir (@searchdirs) {
    print "userinclude $dir\n";
}

%seen = ();

for $dir (@searchdirs) {
    next if $dir =~ /^\s*$/;
    print "sourcepath $dir\n";
    for $file (@files) {
        if (-e "$dir/$file.cpp") {
            print "source $file.cpp\n";
            $seen{$file} = 1;
        }
        if (-e "$dir/$file.c") {
            print "source $file.c\n";
            $seen{$file} = 1;
        }
    }
}

for (@files) {
    die "Did not find source for $_.o!" unless $seen{$_};
}
