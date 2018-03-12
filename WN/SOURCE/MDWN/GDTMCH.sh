
#!/bin/bash

PATH0=/home/beccuti/C++/MDWN/src
PATH1=/home/beccuti/Tools/Smart
i=1
j=0



while [ $i -lt 50 ]
do
        j=0
        while [ $j -lt 10 ];
                do
                $PATH0/GDTMC $1 -H
                $PATH1/smart3-linux-intel64 $1.sm > $1.step
                let j=j+1
        done
        $PATH1/smart3-linux-intel64 $1.sm > $1.rs$i
        echo "****************"
        echo "Step " $i
        echo "****************"
        let i=i+1
done
