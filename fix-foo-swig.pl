#!/usr/bin/perl -ip

use strict;

my $fix1 = quotemeta("if (arg1) (*arg1)->errorHandler_ = *arg2;");
my $fix2 = quotemeta("if (arg1) (arg1)->errorHandler_ = *arg2;");
my $fix3 = quotemeta("result = (Foo::Bar *)new Foo::Bar(arg1);");
my $fix4 = quotemeta("arg1 = *temp;");
    
while (<>) {
    if (/$fix1/) {
	print "if (arg1) (*arg1)->errorHandler_ = std::move(*arg2);\n";
    } elsif (/$fix2/) {
	print "if (arg1) (arg1)->errorHandler_ = std::move(*arg2);\n";
    } elsif (/$fix3/) {
	print "result = (Foo::Bar *)new Foo::Bar(std::move(arg1));\n";
    } elsif (/$fix4/) {
	print "arg1 = std::move(*temp);\n";
    } else {
	print;
    }
}
