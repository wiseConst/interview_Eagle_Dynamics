# Interview

Напишите оптимальный обрабочик столкновений объектов между собой и
краями окна без использования готовых движков симуляции физики. 

Объекты абсолютно упругие, описываются кругами заданного радиуса и 
имеют заданный вектор скорости.

Объекты двигаются в плоскости окна и абсолютно упруго отскакивают 
друг от друга и от краёв окна. 
Массы объектов пропорциональны площади кругов, их описывающих.  

Проект зависит от SFML (https://github.com/SFML/SFML). 
Для OS Windows прикреплены бинарники, для других OS бинарники можно скачать 
с официального сайта https://www.sfml-dev.org/download.php

Алгоритмическая сложность решения должна быть лучше, чем O(n^2).

В предоставленом коде могут встречаться небольшие ошибки и неточности. 
Их надо исправить.
Просьба комментировать код, чтоб было понятно, почему были приняты те или иные решения.

# Tested platforms
- Windows

# Small test, 9999 balls. AMD Ryzen 7 5800H, RTX 3050 Ti Laptop. As you can see in the window, QuadTree implementation is 4 times faster than Naive. First is naive, second is QuadTree + visualization.
![Alt text](/Resources/9999_objects_naive.png)
![Alt text](/Resources/9999_objects_quad_tree.png)

# Clone:
```python
git clone --recursive -b main --single-branch https://github.com/wiseConst/interview_Eagle_Dynamics.git
```
# Build:
```python
cd interview_Eagle_Dynamics && mkdir build && cd build && cmake ..
```
