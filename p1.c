#include "stdio.h"

// Define um valor numerico para cada um dos cinco estados possiveis
enum Estado { INICIAL, INSERIR, LENDO_OP, ENDERECO, TERMINAL };

// Define um valor numerico para cada um dos seis tipos de eventos possiveis
enum TipoDeEvento { ARROBA, L, X, DIGITO, ESPACO, ENTER };

#define MEM_SIZE 0x1000
#define COL_WIDTH 0x10

unsigned char mem[MEM_SIZE];  // memoria de 4096 bytes
int estado = INICIAL;         // estado da maquina de estados
int charEvento = 0;           // char lido da entrada padrao (stdin)
int tipoDeEvento = 0;  // tipo de evento associado ao char (ARROBA, L, X, ...)
int addr = 0;          // posicao de memoria que esta sendo enderecada
int acc = 0;           // utilizado durante a leitura de um byte/endereco
void (*op)() = NULL;   // operacao que deve ser executada no proximo ENTER (\n)
int arg1 = 0;          // primeiro argumento de op

/********************************
 *** FUNCOES DE ENTRADA/SAIDA ***
 ********************************/

// Converte um valor para uma string em notacao hexadecimal
// com o numero especificado de digitos, e imprime essa string.
//
// Exemplo: printHex(0x0F, 3) imprime "00F"
void printHex(int val, int digitos) {
    int digitosAscii[3];
    for (int i = 0; i < digitos; i++) {
        int decimal = val % 0x10;

        if (decimal < 0xA) {
            digitosAscii[i] = decimal + 0x30;  // 0x30 == '0'
        } else {
            digitosAscii[i] = (decimal - 0xA) + 0x41;  // 0x41 == 'A'
        }
        val /= 0x10;
    }
    for (int i = 0; i < digitos; i++) {
        putc(digitosAscii[digitos - i - 1], stdout);
    }
}

/********************************
 ********** OPERACOES ***********
 ********************************/

// Imprime uma mensagem indicando a execucao do endereco dado
//
// Exemplo: opExecutar(0, 0xFFF) imprime "X FFF"
void opExecutar(int _, int codeAddr) {
    putc(0x58, stdout);  // 0x58 == 'X'
    putc(0x20, stdout);  // 0x20 == ' '
    printHex(codeAddr, 3);
    putc(0x0A, stdout);  // 0x0A == '\n'
}

// Imprime os valores armazenados no intervalo de start a end
//
// Exemplo: opListar(0, 32) imprime (dada uma memoria zerada):
//     [000]: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//     [010]: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
void opListar(int start, int end) {
    int i = start;
    while (i <= end) {
        // Imprime o endereco "[XXX]: "
        putc(0x5B, stdout);  // 0x5B == '['
        printHex(i, 3);      // imprime o endereco com 3 digitos
        putc(0x5D, stdout);  // 0x5D == ']'
        putc(0x3A, stdout);  // 0x3A == ':'
        putc(0x20, stdout);  // 0x20 == ' '

        // Imprime os proximos 16 valores (COL_WIDTH==16) separados por espacos
        for (int j = 0; j < COL_WIDTH && i <= end; j++, i++) {
            printHex(mem[i], 2);  // imprime o byte com 2 digitos
            putc(0x20, stdout);   // 0x20 == ' '
        }

        // Comeca uma linha nova para os proximos valores
        putc(0x0A, stdout);  // 0x0A == '\n'
    }
}

// Atualiza addr para o novo valor
void opEnderecar(int _, int newAddr) { addr = newAddr; }

/********************************
 *********** REACOES ************
 ********************************/

// Seta a variavel `op` com base no evento
void reacaoSetOperacao() {
    if (tipoDeEvento == ARROBA) {
        op = opEnderecar;
    } else if (tipoDeEvento == L) {
        op = opListar;
    } else if (tipoDeEvento == X) {
        op = opExecutar;
    }
}

// Atualiza o acumulador com o novo digito
void reacaoAtualizaAcc() {
    int digito;
    if (0x30 <= charEvento && charEvento <= 0x39) {
        digito = charEvento - 0x30;  // (0x30, 0x39) == ('0', '9')
    } else if (0x61 <= charEvento && charEvento <= 0x66) {
        digito = charEvento - 0x61 + 0xA;  // (0x61, 0x66) == ('a', 'f')
    } else {
        digito = charEvento - 0x41 + 0xA;  // 0x41 == 'A'
    }
    acc = 0x10 * acc + digito;
}

// Insere o valor armazenado no acumulador em memoria
void reacaoSetMem() {
    mem[addr] = acc;
    acc = 0;
    addr++;
}

// Seta arg1
void reacaoSetArg1() {
    arg1 = acc;
    acc = 0;
}

// Executa a operacao atual
void reacaoExecutaOperacao() {
    op(arg1, acc);
    acc = 0;
}

// Indica erro de execucao
void reacaoErro() {
    putc(0x65, stdout);  // 0x65 == 'e'
    putc(0x72, stdout);  // 0x72 == 'r'
    putc(0x72, stdout);  // 0x72 == 'r'
    putc(0x6F, stdout);  // 0x6F == 'o'
    putc(0x0A, stdout);  // 0x0A == '\n'
}

typedef struct {
    int proxEstado;  // Proximo estado
    void (*fn)();    // Funcao a ser executada nessa reacao
} Reacao;

// REACOES[s][e] = reacao a partir do estado "s" e evento de tipo "e"
const Reacao REACOES[5][6] = {
    {
        // Estado INICIAL
        /*ARROBA*/ {LENDO_OP, reacaoSetOperacao},
        /*L     */ {ENDERECO, reacaoSetOperacao},
        /*X     */ {ENDERECO, reacaoSetOperacao},
        /*DIGITO*/ {INSERIR, reacaoAtualizaAcc},
        /*ESPACO*/ {INICIAL, NULL},
        /*ENTER */ {INICIAL, NULL},
    },
    {
        // Estado INSERIR
        /*ARROBA*/ {TERMINAL, reacaoErro},
        /*L     */ {TERMINAL, reacaoErro},
        /*X     */ {TERMINAL, reacaoErro},
        /*DIGITO*/ {INSERIR, reacaoAtualizaAcc},
        /*ESPACO*/ {INSERIR, reacaoSetMem},
        /*ENTER */ {INICIAL, reacaoSetMem},
    },
    {
        // Estado LENDO_OP
        /*ARROBA*/ {TERMINAL, NULL},
        /*L     */ {TERMINAL, reacaoErro},
        /*X     */ {TERMINAL, reacaoErro},
        /*DIGITO*/ {ENDERECO, reacaoAtualizaAcc},
        /*ESPACO*/ {ENDERECO, reacaoSetMem},
        /*ENTER */ {TERMINAL, reacaoErro},
    },
    {
        // Estado ENDERECO
        /*ARROBA*/ {TERMINAL, reacaoErro},
        /*L     */ {TERMINAL, reacaoErro},
        /*X     */ {TERMINAL, reacaoErro},
        /*DIGITO*/ {ENDERECO, reacaoAtualizaAcc},
        /*ESPACO*/ {ENDERECO, reacaoSetArg1},
        /*ENTER */ {INICIAL, reacaoExecutaOperacao},
    },
};

/********************************
 ******* CODIGO PRINCIPAL *******
 ********************************/

// Obtem um evento da entrada padrao (stdin).
// Retorna 1 se foi possivel obter um evento, 0 caso contrario.
int extrairEvento() {
    charEvento = getc(stdin);
    if (charEvento == 0x40) {  // 0x40 == '@'
        tipoDeEvento = ARROBA;
    } else if (charEvento == 0x4C) {  // 0x4C == 'L'
        tipoDeEvento = L;
    } else if (charEvento == 0x58) {  // 0x58 == 'X'
        tipoDeEvento = X;
    } else if (charEvento == 0x0A) {  // 0x0A == '\n'
        tipoDeEvento = ENTER;
    } else if (charEvento == 0x20) {  // 0x20 == ' '
        tipoDeEvento = ESPACO;
    } else if ((0x30 <= charEvento && charEvento <= 0x39) ||  // digitos 0-9
               (0x61 <= charEvento && charEvento <= 0x66) ||  // digitos a-f
               (0x41 <= charEvento && charEvento <= 0x46)) {  // digitos A-F
        tipoDeEvento = DIGITO;
    } else {
        return 0;
    }
    return 1;
}

int main() {
    while (estado != TERMINAL) {
        if (!extrairEvento()) {
            // Termina o programa se nao for possivel ler o evento
            reacaoErro();
            break;
        }

        // Obtem o proximo estado e a funcao a ser executada
        const Reacao *r = &REACOES[estado][tipoDeEvento];

        if (r->fn != NULL) {
            r->fn();  // Executa a funcao, caso ela existaa
        }
        estado = r->proxEstado;  // Atualiza o estado
    }

    putc(0x0A, stdout);      // 0x0A == '\n'
    opListar(0x000, 0xFFF);  // Imprime o conteudo completo da memoria
}
