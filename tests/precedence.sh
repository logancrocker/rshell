(echo A && echo B) || (echo C && echo D)
(echo K; echo O) && (echo P || echo T)
(echo A || echo B) && (echo C || echo D)
(false || echo 1 && echo 3) && (echo 6 || echo 7)
([makefile] && echo 1) || echo 2
((test -d src || echo 1) && echo 2) && echo 3
([-e asdfasdf] || echo 1) && echo 2 && echo 3
