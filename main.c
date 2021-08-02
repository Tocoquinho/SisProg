#include "stdio.h"

// Define um valor numerico para cada um dos cinco estados possiveis
enum Estado { INICIAL, NUM1, NUM2, REG1, REG1A, REG2, REG2A, NUM2REG2, ESPERA, TERMINAL };

// Define um valor numerico para cada um dos seis tipos de eventos possiveis
enum TipoDeEvento { DIGITO, OP_ARIT, IGUAL, R, TIL, X, ENTER, P };

#define NUM_REG 10
#define NUM_LINHAS 0x1024
#define COMPRIMENTO_MAX_LINHA 0x32

unsigned char mem[NUM_LINHAS*COMPRIMENTO_MAX_LINHA];  // memoria programável
unsigned int regs[NUM_REG]; // registradores
int estado = INICIAL;  // estado da maquina de estados
int charEvento = 0;    // char lido da entrada padrao (stdin)
int tipoDeEvento = 0;  // tipo de evento associado ao char (ARROBA, L, X, ...)

int operacao = 0;    // operacao a ser realizada (+,-,*,/)
int acumulador = 0;  //
int acumuladorInterno = 0;  // usado na leitura de numeros do usuario
int operando1 = 0;   // primeiro operando
int operando2 = 0;   // segundo operando
int ulitmoReg = 0;   // guarda o ultimo registrador referenciado em algum operando

/********************************
 *********** REACOES ************
 ********************************/

// Indica erro de execucao
void reacaoErro() {
    printf("erro\n");
}

// Atualiza o acumulador com o número fornecido
void reacaoAtualizaAcumulador() {
    acumulador = acumuladorInterno;
    acumuladorInterno = 0;
    printf("> %d\n\n", acumulador);
}

// Armazena o digito fornecido
void reacaoLerDigito() {
    acumuladorInterno = acumuladorInterno*10 + (charEvento - '0');
}

// Armazena o valor do registrador especificado
void reacaoLerRegistrador() {
    ulitmoReg = charEvento - '0';
    acumuladorInterno = regs[ulitmoReg];
}


// Armazena a operação fornecida
void reacaoLerOperacao() {
    operacao = charEvento;
}

// Armazena a operação fornecida e o numero obtido no acumulador interno
void reacaoLerOperacaoSalvarNum() {
    reacaoLerOperacao();
    operando1 = acumuladorInterno;
    acumuladorInterno = 0;
}

// Armazena a operação fornecida e atribui o valor do operando 1
void reacaoLerOperacaoAtualizarOperando() {
    reacaoLerOperacao();
    operando1 = acumulador;
}

// Armazena o valor do operando 2 e realiza a operação desejada
void reacaoRealizaOperacao() {
    operando2 = acumuladorInterno;
    acumuladorInterno = 0;

    if(operacao == '+'){
        acumulador = operando1 + operando2;
    }
    else if(operacao == '-'){
        acumulador = operando1 - operando2;
    }
    else if(operacao == '*'){
        acumulador = operando1 * operando2;
    }
    else if(operacao == '/'){
        acumulador = operando1 / operando2;
    }
    else if(operacao == '='){
        regs[ulitmoReg] = operando1;
    }
    // printf("%d %d\n", operando1, operando2);
    printf("> %d\n\n", acumulador);
}

typedef struct {
    int proxEstado;  // Proximo estado
    void (*fn)();    // Funcao a ser executada nessa reacao
} Reacao;

// REACOES[s][e] = reacao a partir do estado "s" e evento de tipo "e"
const Reacao REACOES[10][8] = {
    {
        // Estado INICIAL
        /*DIGITO */ {NUM1, reacaoLerDigito},
        /*OP_ARIT*/ {NUM2REG2, reacaoLerOperacaoAtualizarOperando},
        /*IGUAL  */ {NUM2REG2, reacaoLerOperacaoAtualizarOperando},
        /*R      */ {REG1, NULL},
        /*TIL    */ {ESPERA, NULL},
        /*X      */ {ESPERA, NULL},
        /*ENTER  */ {INICIAL, NULL},
        /*P      */ {INICIAL, NULL},
    },
    {
        // Estado NUM1
        /*DIGITO */ {NUM1, reacaoLerDigito},
        /*OP_ARIT*/ {NUM2REG2, reacaoLerOperacaoSalvarNum},
        /*IGUAL  */ {TERMINAL, NULL},
        /*R      */ {TERMINAL, reacaoErro},
        /*TIL    */ {TERMINAL, reacaoErro},
        /*X      */ {TERMINAL, reacaoErro},
        /*ENTER  */ {INICIAL, reacaoAtualizaAcumulador},
        /*P      */ {TERMINAL, reacaoErro},
    },
    {
        // Estado NUM2
        /*DIGITO */ {NUM2, reacaoLerDigito},
        /*OP_ARIT*/ {TERMINAL, reacaoErro},
        /*IGUAL  */ {TERMINAL, NULL},
        /*R      */ {TERMINAL, reacaoErro},
        /*TIL    */ {TERMINAL, reacaoErro},
        /*X      */ {TERMINAL, reacaoErro},
        /*ENTER  */ {INICIAL, reacaoRealizaOperacao},
        /*P      */ {TERMINAL, reacaoErro},
    },
    {
        // Estado REG1
        /*DIGITO */ {REG1A, reacaoLerRegistrador},
        /*OP_ARIT*/ {TERMINAL, reacaoErro},
        /*IGUAL  */ {TERMINAL, NULL},
        /*R      */ {TERMINAL, reacaoErro},
        /*TIL    */ {TERMINAL, reacaoErro},
        /*X      */ {TERMINAL, reacaoErro},
        /*ENTER  */ {TERMINAL, reacaoErro},
        /*P      */ {TERMINAL, reacaoErro},
    },
    {
        // Estado REG1A
        /*DIGITO */ {TERMINAL, reacaoErro},
        /*OP_ARIT*/ {NUM2REG2, reacaoLerOperacaoSalvarNum},
        /*IGUAL  */ {TERMINAL, NULL},
        /*R      */ {TERMINAL, reacaoErro},
        /*TIL    */ {TERMINAL, reacaoErro},
        /*X      */ {TERMINAL, reacaoErro},
        /*ENTER  */ {INICIAL, reacaoAtualizaAcumulador},
        /*P      */ {TERMINAL, reacaoErro},
    },
    {
        // Estado REG2
        /*DIGITO */ {REG2A, reacaoLerRegistrador},
        /*OP_ARIT*/ {TERMINAL, reacaoErro},
        /*IGUAL  */ {TERMINAL, NULL},
        /*R      */ {TERMINAL, reacaoErro},
        /*TIL    */ {TERMINAL, reacaoErro},
        /*X      */ {TERMINAL, reacaoErro},
        /*ENTER  */ {TERMINAL, reacaoErro},
        /*P      */ {TERMINAL, reacaoErro},
    },
    {
        // Estado REG2A
        /*DIGITO */ {TERMINAL, reacaoErro},
        /*OP_ARIT*/ {TERMINAL, reacaoErro},
        /*IGUAL  */ {TERMINAL, NULL},
        /*R      */ {TERMINAL, reacaoErro},
        /*TIL    */ {TERMINAL, reacaoErro},
        /*X      */ {TERMINAL, reacaoErro},
        /*ENTER  */ {INICIAL, reacaoRealizaOperacao},
        /*P      */ {TERMINAL, reacaoErro},
    },
        {
        // Estado NUM2REG2
        /*DIGITO */ {NUM2, reacaoLerDigito},
        /*OP_ARIT*/ {TERMINAL, reacaoErro},
        /*IGUAL  */ {TERMINAL, NULL},
        /*R      */ {REG2, NULL},
        /*TIL    */ {TERMINAL, reacaoErro},
        /*X      */ {TERMINAL, reacaoErro},
        /*ENTER  */ {TERMINAL, reacaoErro},
        /*P      */ {TERMINAL, reacaoErro},
    },
    {
        // Estado ESPERA
        /*DIGITO */ {TERMINAL, reacaoErro},
        /*OP_ARIT*/ {TERMINAL, reacaoErro},
        /*IGUAL  */ {TERMINAL, NULL},
        /*R      */ {TERMINAL, reacaoErro},
        /*TIL    */ {TERMINAL, reacaoErro},
        /*X      */ {TERMINAL, reacaoErro},
        /*ENTER  */ {INICIAL, NULL},
        /*P      */ {TERMINAL, reacaoErro},
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
        tipoDeEvento = OP_ARIT;
    } else if (charEvento == '=') {
        tipoDeEvento = IGUAL;
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
