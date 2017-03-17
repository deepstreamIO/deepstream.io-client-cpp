#!/usr/bin/perl -i

use strict;

my $fix1 = quotemeta("arg2 = *temp;");
my $fix2 = quotemeta("result = (deepstream::Client *)new deepstream::Client((std::string const &)*arg1,arg2);");
my $fix3 = quotemeta("result = (deepstream::Client *)new deepstream::Client((std::string const &)*arg1,arg2,arg3);");

while (<>) {
    if (/$fix1/) {
	print "arg2 = std::move(*temp);\n";
    } elsif (/$fix2/) {
	print "result = (deepstream::Client *)new deepstream::Client((std::string const &)*arg1,std::move(arg2));\n";
    } elsif (/$fix3/) {
	print "result = (deepstream::Client *)new deepstream::Client((std::string const &)*arg1,std::move(arg2),arg3);\n";
    } else {
	print;
    }
}
