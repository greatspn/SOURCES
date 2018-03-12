#Recupera i risultati generati da smart e genera un input per gnuplot

#!/usr/bin/ruby -w





$i=0

def read_open(f)

	end
end

def show_regexp(a, re)
	if a =~ re
  		$`
	else
		a
	end
end




if ARGV[0]==nil then
	print "\nError: use name <path/netname>\n\n"
else


	PathDir=show_regexp(ARGV[0],/.\w+$/)
	begin
	read_open(f,ARGV[1])
	files.each{|f| line=line+"\n"+read_open(f)}
	fp=File.open("#{PathDir}.data","w")
	fp.puts line
	fp.close
		rescue SystemCallError
			$stderr.print "\nError: "+PathDir+" is not a valid path\n\n"
		exit(1)

	end

end
