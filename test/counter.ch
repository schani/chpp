%define(newcounter,%locals(n,%<n=0>%lambda(%<n=%[n+1]>%n)))

%<counter1=%newcounter()>
%<counter2=%newcounter()>

%counter1()  %counter1()  %counter1()

%counter2()  %counter2()  %counter2()
