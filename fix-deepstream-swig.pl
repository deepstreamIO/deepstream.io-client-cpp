#!/usr/bin/perl -ip

use strict;

my $fix1 = quotemeta("arg2 = *temp;");
my $fix2 = quotemeta("result = (deepstream::Client *)new deepstream::Client((std::string const &)*arg1,arg2);");
    
while (<>) {
    if (/$fix1/) {
	print "arg2 = std::move(*temp);\n";
    } elsif (/$fix2/) {
	print "result = (deepstream::Client *)new deepstream::Client((std::string const &)*arg1,std::move(arg2));\n";
    } else {
	print;
    }
}
