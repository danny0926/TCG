# TCG-Threes!
A 2048 like game : Threes!
We use some heuristic functions to implement.

## basic usage
Making the sample program.
```
make # see makefile for details
```

Running the sample program.
```
./threes
```

Training
```
./threes --total=10000 --limit=1000 --block=1000 --slide="init=16777216,16777216,16777216,16777216,16777216 alpha=0.003 load=weight.bin save=weight.bin" place="seed=12345"
```
Testing
```
./threes --total=1000 save=stats.txt
```

## Author
[Computer Games and Intelligence (CGI) Lab](https://cgilab.nctu.edu.tw/), NYCU, Taiwan
