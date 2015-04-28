find . -name "*.c" | gawk '{print "gcc " $0 " -o " substr($0, 0 , length($0) - 2)}' | bash
