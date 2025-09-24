# MAZE OF LOST SOULS
Um jogo de exploração e suspense em primeira pessoa.

**Colete. Fuja. Sobreviva.**

---

![Status](https://img.shields.io/badge/status-Concluído-darkgreen?style=for-the-badge)
![Linguagem](https://img.shields.io/badge/linguagem-C-darkblue?style=for-the-badge&logo=c)
![OpenGL](https://img.shields.io/badge/OpenGL-3D%20Graphics-darkred?style=for-the-badge&logo=opengl)
![Plataforma](https://img.shields.io/badge/plataforma-PC-yellow?style=for-the-badge&logo=windows)

---

## Ideia e Conceito

**MAZE OF LOST SOULS** é um jogo interativo de exploração e suspense em primeira pessoa. A proposta é criar uma experiência atmosférica e tensa, onde o jogador deve navegar por um labirinto escuro e claustrofóbico. A inspiração inicial veio de jogos como **"Dark Deception"**, que modernizam a fórmula de Pac-Man em uma perspectiva de terror.

Para este projeto, o conceito foi adaptado: a mecânica de perseguição foi removida para focar nas técnicas gráficas de criação de atmosfera. O jogo se tornou uma experiência de coleta e fuga contra o tempo, onde a direção de arte foi inspirada na estética **"Backrooms"**, buscando um ambiente opressivo através da iluminação e do design do labirinto.

## Demonstração do Jogo

O jogo é dividido em duas fases distintas, com um objetivo claro para o jogador.

* **Fase 1: Coleta:** O jogador começa em um labirinto escuro e precisa encontrar e coletar todas as esferas brilhantes que estão espalhadas pelo cenário. A iluminação ambiente se torna progressivamente mais avermelhada a cada coleta, aumentando a tensão.

* **Fase 2: Fuga:** Após coletar a última esfera, a saída do labirinto é revelada. Um cronômetro de contagem regressiva é iniciado, e o jogador deve encontrar a saída (um buraco no chão) e escapar antes que o tempo se esgote.

### Controles e Interação

A interação com o jogo é feita através do teclado e mouse:

* **Movimentação:**
    * `W`: Mover para frente.
    * `S`: Mover para trás.
    * `A`: Mover para a esquerda (strafe).
    * `D`: Mover para a direita (strafe).
* **Câmera:**
    * `Mouse`: Controla a direção da visão em 360°, permitindo olhar para cima, para baixo e para os lados.
* **Pausa:**
    * `ESC`: Pausa o jogo a qualquer momento e abre o menu de pausa, que oferece as opções de continuar, voltar ao menu principal ou encerrar o jogo.

### Customização do Labirinto

O layout do labirinto é totalmente customizável de forma simples, editando uma matriz 2D no arquivo `game.c`.

A matriz `maze_grid` define o mapa, onde cada número representa um elemento diferente:

* `0`: Caminho livre.
* `1`: Parede.
* `2`: Esfera coletável.
* `9`: Posição da saída. Funciona como uma parede na Fase 1 e se torna a base para o "buraco" na Fase 2.

**Exemplo de um trecho do mapa:**
```c
// Em game.c
static int maze_grid[MAZE_WIDTH][MAZE_HEIGHT] = {
    {1,1,1,1,1,...},
    {1,2,0,1,0,...}, // Posição (1,1) tem um coletável (2)
    {1,1,0,1,1,...},
    //...
    {...,0,0,0,9,1}, // Posição (13,13) é a saída (9)
    {...,1,1,1,1,1}
};
```

## Implementação e Lógica

O código do projeto foi inteiramente desenvolvido em **Linguagem C** e modularizado para facilitar a organização e manutenção. A estrutura é dividida nos seguintes módulos:

* `main.c`: Ponto de entrada da aplicação.
* `game.c`: Cérebro do jogo, contendo a máquina de estados.
* `player.c`: Lógica de movimentação, câmera e colisão.
* `render.c`: Lógica gráfica e de iluminação.
* `ui.c`: Menus, botões e interface.
* `texture.c`: Carregamento de texturas.

A lógica de estados é o pilar do funcionamento do jogo, onde cada estado (`STATE_MAIN_MENU`, `STATE_PLAYING`, `STATE_ESCAPING`, etc.) dita quais funções de atualização e renderização devem ser executadas.

## Técnicas de Computação Gráfica

* **Modelagem 3D:** O mundo do jogo é construído a partir de primitivas 3D do OpenGL: cubos (`glutSolidCube`), esferas (`gluSphere`) e planos (`glBegin(GL_QUADS)`).

* **Câmera:** Foi implementada uma câmera em primeira pessoa funcional, controlada pelo mouse (`yaw` e `pitch`) e posicionada com `gluLookAt`.

* **Iluminação:** A iluminação é o elemento principal da atmosfera. O jogo utiliza uma luz ambiente dinâmica (`glLightModelfv`), luzes pontuais (`glLightfv`) para as esferas e para a saída, e materiais emissivos (`GL_EMISSION`) para fazer os objetos brilharem.

* **Projeção:** O jogo utiliza **projeção perspectiva** (`gluPerspective`) para a cena 3D e **projeção ortogonal** (`gluOrtho2D`) para a interface 2D.

* **Remoção de Superfícies Ocultas:** A técnica está ativa através do `glEnable(GL_DEPTH_TEST)`.

## Recursos Extras

* **Adição de Texturas:** O jogo faz uso extensivo de texturas para paredes, chão, teto, colecionáveis e interfaces de menu, carregadas de arquivos `.bmp`.

* **Importação de Objetos e Projeção de Sombras:** Estes recursos avançados não foram implementados na versão atual.

## Desafios e Soluções

* **Iluminação Dinâmica:** A limitação de luzes do OpenGL antigo foi contornada com um sistema "fake", onde uma única luz pontual se move para o colecionável mais próximo do jogador.

* **Interface Gráfica em GLUT:** Um sistema de UI foi construído do zero, com lógica para desenhar botões e detectar cliques do mouse em modo de projeção ortogonal.
