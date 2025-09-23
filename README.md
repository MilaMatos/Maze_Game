# Jogo: MAZE OF LOST SOULS

## Ideia e Conceito

**MAZE OF LOST SOULS** é um jogo interativo de exploração e suspense em primeira pessoa. A proposta é criar uma experiência atmosférica e tensa, onde o jogador deve navegar por um labirinto escuro e claustrofóbico. A principal inspiração para o tema foi a estética "Backrooms", que se foca em ambientes liminares, desorientação e um sutil terror psicológico, onde a iluminação e a ambientação são os elementos centrais para criar a imersão. O objetivo não é o combate, mas a exploração sob pressão, utilizando a luz como guia e o tempo como adversário.

## Demonstração do Jogo

O jogo é dividido em duas fases distintas, com um objetivo claro para o jogador.

* **Fase 1: Coleta:** O jogador começa em um labirinto escuro e precisa encontrar e coletar todas as esferas brilhantes que estão espalhadas pelo cenário. A única iluminação significativa vem dessas próprias esferas, forçando o jogador a se mover de um ponto de luz a outro. A cada esfera coletada, a luz ambiente do labirinto se torna sutilmente mais avermelhada, aumentando a tensão.

* **Fase 2: Fuga:** Após coletar a última esfera, a saída do labirinto é revelada e uma luz se acende sobre ela, indicando o caminho. Um cronômetro de contagem regressiva é iniciado, e o jogador deve encontrar a saída e escapar antes que o tempo se esgote. A saída é um buraco no chão, e para vencer, o jogador deve cair nele.

O jogo conta com um menu principal, um menu de pausa (ativado pela tecla ESC), e telas de vitória e derrota, todos com botões clicáveis e imagens de fundo para uma experiência mais completa.

## Implementação e Lógica

O código do projeto foi inteiramente desenvolvido em **Linguagem C** e modularizado para facilitar a organização e manutenção. A estrutura é dividida nos seguintes módulos:

* `main.c`: Ponto de entrada da aplicação, responsável por inicializar o GLUT, registrar as funções de callback e iniciar o loop principal do jogo.
* `game.c`: O cérebro do projeto. Contém a **máquina de estados** que gerencia o fluxo do jogo (menu, jogando, pausado, vitória, derrota). Ele controla a lógica de transição de fases, as condições de vitória/derrota e o cronômetro.
* `player.c`: Encapsula toda a lógica do jogador, incluindo a `struct Player`, o processamento de input do teclado (WASD) para movimentação, a lógica da câmera com o mouse e a detecção de colisão com as paredes do labirinto.
* `render.c`: Responsável por toda a parte gráfica. Contém as funções para desenhar o labirinto, o chão, o teto, os colecionáveis e a saída. Também gerencia a inicialização e atualização das fontes de luz.
* `ui.c`: Implementa toda a interface do usuário, como o desenho dos menus, botões, HUD (heads-up display) e a lógica para detectar cliques do mouse.
* `texture.c`: Um módulo auxiliar que contém a função para carregar texturas no formato `.bmp`.
* `config.h`: Arquivo de configuração central que define constantes globais (`MAZE_WIDTH`, `PLAYER_SIZE`, etc.) e as `structs` e `enums` principais.

A lógica de estados é o pilar do funcionamento do jogo, onde cada estado (`STATE_MAIN_MENU`, `STATE_PLAYING`, `STATE_ESCAPING`, etc.) dita quais funções de atualização e renderização devem ser executadas a cada quadro.

## Técnicas de Computação Gráfica

* **Modelagem 3D:** O mundo do jogo é construído a partir de primitivas 3D do OpenGL. As paredes são cubos (`glutSolidCube`) escalados, o chão e o teto são planos (`glBegin(GL_QUADS)`), e os colecionáveis e o marcador da saída são esferas (`gluSphere`).

* **Câmera:** Foi implementada uma câmera em primeira pessoa totalmente funcional. A posição da câmera está vinculada às coordenadas `(x, y, z)` do jogador, e sua orientação é controlada pelo mouse. A direção para onde a câmera aponta é calculada dinamicamente usando trigonometria (`cos`, `sin`) com base nos ângulos de rotação horizontal (`yaw`) e vertical (`pitch`), e aplicada através da função `gluLookAt`.

* **Iluminação:** A iluminação é o elemento principal da atmosfera do jogo.
    * **Luz Ambiente:** O jogo utiliza uma luz ambiente global dinâmica, configurada via `glLightModelfv`. A cor dessa luz é atualizada em tempo real pela função `render_update_ambient_light`, que a torna progressivamente mais vermelha a cada esfera coletada, intensificando o clima.
    * **Luz Dinâmica (Pontual):** São usadas duas fontes de luz pontuais (`GL_LIGHT1` e `GL_LIGHT2`). A `GL_LIGHT1` é dinâmica: sua posição é atualizada a cada quadro para a localização da esfera coletável mais próxima do jogador, criando o efeito de que as esferas iluminam o caminho. A `GL_LIGHT2` é ativada sobre a saída apenas na fase de fuga.
    * **Emissão:** As esferas coletáveis e a esfera da saída possuem um material com alta emissão (`glMaterialfv` com `GL_EMISSION`), fazendo com que elas brilhem no escuro e funcionem como "faróis" no labirinto.
    * **Componentes Difusa e Especular:** Todas as fontes de luz e materiais utilizam propriedades difusas e especulares para criar um sombreamento realista nas superfícies e realces de brilho, respectivamente.

* **Projeção:** O jogo utiliza duas projeções distintas. A cena 3D do labirinto usa uma **projeção perspectiva** (`gluPerspective`) para dar a sensação de profundidade. Já a interface 2D (menus e HUD) é desenhada usando uma **projeção ortogonal** (`gluOrtho2D`), garantindo que os elementos fiquem "colados" na tela.

* **Remoção de Superfícies Ocultas:** A técnica está ativa através do `glEnable(GL_DEPTH_TEST)`, que garante que os polígonos sejam desenhados na ordem correta de profundidade, evitando que uma parede distante apareça na frente de uma próxima.

## Recursos Extras

* **Adição de Texturas:** O jogo faz uso extensivo de texturas para aumentar o realismo e a imersão. São aplicadas texturas nas paredes, no chão, no teto, nos colecionáveis e na esfera da saída. Além disso, os menus e telas de vitória/derrota possuem imagens de fundo personalizadas. Todas as texturas são carregadas de arquivos `.bmp`.

* **Importação de Objetos e Projeção de Sombras:** Estes recursos avançados não foram implementados na versão atual do projeto.

## Desafios e Soluções

* **Iluminação Dinâmica:** A limitação do OpenGL antigo a um pequeno número de fontes de luz (geralmente 8) impedia que cada esfera coletável fosse uma fonte de luz real. A solução foi um "truque": todas as esferas possuem um material emissivo para brilhar, mas apenas uma fonte de luz pontual (`GL_LIGHT1`) é ativada dinamicamente, tendo sua posição alterada a cada quadro para a da esfera mais próxima do jogador.

* **Interface Gráfica em GLUT:** A biblioteca GLUT não oferece componentes de UI prontos. O desafio foi criar um sistema de menus e botões clicáveis do zero. A solução foi implementar uma `struct Button`, desenhar os elementos com primitivas 2D em modo de projeção ortogonal, e criar uma função que converte as coordenadas do clique do mouse (origem no topo) para as coordenadas do OpenGL (origem embaixo) para detectar a interação.
