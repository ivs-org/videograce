set server="192.168.1.100:8778"
set conference="default"
set play_file="play.mkv"
set count=10

for /L %%B in (1,1,%count%) do start ShClnt.exe %server% test%%B 1 %conference% %play_file%