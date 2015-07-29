



alarm : main.o menu0.o menu1.o menu2.o menu3.o menu4.o menu5.o menu6.o menu7.o menu8.o menu9.o menu10.o menu11.o menu12.o menu15.o menu20.o menu21.o configfile.o
	cc -o alarm main.o menu0.o menu1.o menu2.o menu3.o menu4.o menu5.o menu6.o menu7.o menu8.o menu9.o menu10.o menu11.o menu12.o menu15.o menu20.o menu21.o configfile.o -liniparser -lpthread -ldispsw -lwiringPi

main.o : main.c main.h types.h configfile.h
	cc -Wall -c main.c

menu0.o : menu0.c main.h types.h configfile.h
	cc -Wall -c menu0.c

menu1.o : menu1.c main.h types.h configfile.h
	cc -Wall -c menu1.c

menu2.o : menu2.c main.h types.h configfile.h
	cc -Wall -c menu2.c

menu3.o : menu3.c main.h types.h configfile.h
	cc -Wall -c menu3.c

menu4.o : menu4.c main.h types.h configfile.h
	cc -Wall -c menu4.c

menu5.o : menu5.c main.h types.h configfile.h
	cc -Wall -c menu5.c

menu6.o : menu6.c main.h types.h configfile.h
	cc -Wall -c menu6.c

menu7.o : menu7.c main.h types.h configfile.h
	cc -Wall -c menu7.c

menu8.o : menu8.c main.h types.h configfile.h
	cc -Wall -c menu8.c

menu9.o : menu9.c main.h types.h configfile.h
	cc -Wall -c menu9.c

menu10.o : menu10.c main.h types.h configfile.h
	cc -Wall -c menu10.c

menu11.o : menu11.c main.h types.h  configfile.h
	cc -Wall -c menu11.c

menu12.o : menu12.c main.h types.h  configfile.h
	cc -Wall -c menu12.c

menu15.o : menu15.c main.h types.h  configfile.h
	cc -Wall -c menu15.c

menu20.o : menu20.c main.h types.h  configfile.h
	cc -Wall -c menu20.c

menu21.o : menu21.c main.h types.h  configfile.h
	cc -Wall -c menu21.c

configfile.o : configfile.c main.h types.h configfile.h
	cc -Wall -c configfile.c



