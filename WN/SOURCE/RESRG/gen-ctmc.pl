#!/usr/bin/env perl

use warnings;
use strict;


foreach (<>)
{
    if( /color=\w+, label=/ ){
       chomp $_ ;
       print $_ ."\\n";
   }
    elsif (/\|\w+\|=\w+/){
       chomp $_ ;
       print $_ ;
   }else{
       print $_;
   }
}

