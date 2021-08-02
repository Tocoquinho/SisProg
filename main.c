#include "stdio.h"

// Define um valor numerico para cada um dos cinco estados possiveis
enum Estado { INICIAL, NUM1, NUM2, REG1, REG1A, REG2, REG2A, NUM2REG2, ESPERA, TERMINAL };

// Define um valor numerico para cada um dos seis tipos de eventos possiveis
enum TipoDeEvento { DIGITO, OP, R, TIL, X, ENTER, P };

#define NUM_REG 10
#define NUM_LINHAS 0x1024
#define COMPRIMENTO_MAX_LINHA 0x32

unsigned char mem[NUM_LINHAS*COMPRIMENTO_MAX_LINHA];  // memoria programável
int estado = INICIAL;  // estado da maquina de estados
int charEvento = 0;    // char lido da entrada padrao (stdin)
int tipoDeEvento = 0;  // tipo de evento associado ao char (ARROBA, L, X, ...)

int operacao = 0;    // operacao a ser realizada (+,-,*,/)
int acumulador = 0;  //
int acumuladorInterno = 0;  // usado na leitura de numeros do usuario
int operando1 = 0;   // primeiro operando
int operando2 = 0;   // segundo operando

/********************************
 *********** REACOES ************
 ********************************/

// Indica erro de execucao
void reacaoErro() {
    printf("erro\n");
}

// Indica erro de execucao
void reacaoLerDigito() {
    acumuladorInterno += charEvento - '0';
}

typedef struct {
    int proxEstado;  // Proximo estado
    void (*fn)();    // Funcao a ser executada nessa reacao
} Reacao;

// REACOES[s][e] = reacao a partir do estado "s" e evento de tipo "e"
const Reacao REACOES[5][6] = {
    {
        // Estado INICIAL
        /*DIGITO*/ {NUM1, reacaoLerDigito},
        /*OP    */ {NUM2REG2, NULL},
        /*R     */ {REG1, NULL},
        /*TIL   */ {ESPERA, NULL},
        /*X     */ {ESPERA, NULL},
        /*ENTER */ {INICIAL, NULL},
        /*P     */ {INICIAL, NULL},
    },
    {
        // Estado NUM1
        /*DIGITO*/ {NUM1, reacaoLerDigito},
        /*OP    */ {NUM2REG2, NULL},
        /*R     */ {TERMINAL, reacaoErro},
        /*TIL   */ {TERMINAL, reacaoErro},
        /*X     */ {TERMINAL, reacaoErro},
        /*ENTER */ {INICIAL, NULL},
        /*P     */ {TERMINAL, reacaoErro},
    },
    {
        // Estado NUM2
        /*DIGITO*/ {NUM2, reacaoLerDigito},
        /*OP    */ {TERMINAL, reacaoErro},
        /*R     */ {TERMINAL, reacaoErro},
        /*TIL   */ {TERMINAL, reacaoErro},
        /*X     */ {TERMINAL, reacaoErro},
        /*ENTER */ {INICIAL, NULL},
        /*P     */ {TERMINAL, reacaoErro},
    },
    {
        // Estado REG1
        /*DIGITO*/ {REG1A, reacaoLerDigito},
        /*OP    */ {TERMINAL, reacaoErro},
        /*R     */ {TERMINAL, reacaoErro},
        /*TIL   */ {TERMINAL, reacaoErro},
        /*X     */ {TERMINAL, reacaoErro},
        /*ENTER */ {TERMINAL, reacaoErro},
        /*P     */ {TERMINAL, reacaoErro},
    },
        {
        // Estado REG1A
        /*DIGITO*/ {TERMINAL, reacaoErro},
        /*OP    */ {NUM2REG2, NULL},
        /*R     */ {TERMINAL, reacaoErro},
        /*TIL   */ {TERMINAL, reacaoErro},
        /*X     */ {TERMINAL, reacaoErro},
        /*ENTER */ {TERMINAL, reacaoErro},
        /*P     */ {TERMINAL, reacaoErro},
    },
        {
        // Estado REG2
        /*DIGITO*/ {REG2A, reacaoLerDigito},
        /*OP    */ {TERMINAL, reacaoErro},
        /*R     */ {TERMINAL, reacaoErro},
        /*TIL   */ {TERMINAL, reacaoErro},
        /*X     */ {TERMINAL, reacaoErro},
        /*ENTER */ {TERMINAL, reacaoErro},
        /*P     */ {TERMINAL, reacaoErro},
    },
        {
        // Estado REG2A
        /*DIGITO*/ {TERMINAL, reacaoErro},
        /*OP    */ {TERMINAL, reacaoErro},
        /*R     */ {TERMINAL, reacaoErro},
        /*TIL   */ {TERMINAL, reacaoErro},
        /*X     */ {TERMINAL, reacaoErro},
        /*ENTER */ {INICIAL, NULL},
        /*P     */ {TERMINAL, reacaoErro},
    },
        {
        // Estado NUM2REG2
        /*DIGITO*/ {NUM2, reacaoLerDigito},
        /*OP    */ {TERMINAL, reacaoErro},
        /*R     */ {REG2, NULL},
        /*TIL   */ {TERMINAL, reacaoErro},
        /*X     */ {TERMINAL, reacaoErro},
        /*ENTER */ {TERMINAL, reacaoErro},
        /*P     */ {TERMINAL, reacaoErro},
    },
        {
        // Estado ESPERA
        /*DIGITO*/ {TERMINAL, reacaoErro},
        /*OP    */ {TERMINAL, reacaoErro},
        /*R     */ {TERMINAL, reacaoErro},
        /*TIL   */ {TERMINAL, reacaoErro},
        /*X     */ {TERMINAL, reacaoErro},
        /*ENTER */ {INICIAL, NULL},
        /*P     */ {TERMINAL, reacaoErro},
    },
};

/********************************
 ******* MOTOR DE EVENTOS *******
 ********************************/

// Obtem um evento da entrada padrao (stdin).
// Retorna 1 se foi possivel obter um evento, 0 caso contrario.
int extrairEvento() {
    charEvento = getc(stdin);
    if (('0' <= charEvento && charEvento <= '9')) {  // digitos 0-9
        tipoDeEvento = DIGITO;
    } else if (charEvento == '+' || charEvento == '-' ||
               charEvento == '*' || charEvento == '/') {
        tipoDeEvento = OP;
    } else if (charEvento == 'R') {
        tipoDeEvento = R;
    } else if (charEvento == '~') {
        tipoDeEvento = TIL;
    } else if (charEvento == 'X') {
        tipoDeEvento = X;
    } else if (charEvento == '\n') {
        tipoDeEvento = ENTER;
    } else if (charEvento == 'P') {
        tipoDeEvento = P;
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
            r->fn();  // Executa a funcao, caso ela exista
        }
        estado = r->proxEstado;  // Atualiza o estado
    }
}
