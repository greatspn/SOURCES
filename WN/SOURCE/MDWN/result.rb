#Recupera i risultati generati da smart e genera un input per gnuplot

#!/usr/bin/ruby -w


$i=0

def read_open(f)
	File.open(f,"r") do |file|
		file.gets
		file.gets
		tmp=file.gets.chomp.gsub(/Prob.: /,'')
		line="#{tmp} #{$i}"
		$i=$i+1
		return line
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
	print "\nError: use plotRes <path/netname>\n\n"
else
	PathDir=show_regexp(ARGV[0],/.\w+$/)
	begin
	line="0 0"
	puts PathDir
	files=Dir.glob("#{PathDir}.rs*") 
	puts files 
	files.each{|f| line=line+"\n"+read_open(f)}
	fp=File.open("#{PathDir}.data","w")
	fp.puts line
	fp.close
		rescue SystemCallError
			$stderr.print "\nError: "+PathDir+" is not a valid path\n\n"
		exit(1)

	end

end
