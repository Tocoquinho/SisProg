#include "stdio.h"

// Define um valor numerico para cada um dos cinco estados possiveis
enum Estado {
    INICIO,
    NUM1,
    NUM2,
    REG1,
    REG1A,
    REG2,
    REG2A,
    NUM2REG2,
    ESPERA,
    SET,
    BP,
    PB,
    P_INICIO,
    P_CODIGO,
    P_ARG,
    P_LISTAR,
    TERMINAL
};

// Define um valor numerico para cada um dos seis tipos de eventos possiveis
enum TipoDeEvento {
    DIGITO,
    OP_ARIT,
    IGUAL,
    R,
    TIL,
    X,
    ENTER,
    P,
    L,
    E_COMERCIAL,
    OUTROS
};

#define NUM_REG 10
#define NUM_LINHAS 0x1024
#define COMPRIMENTO_MAX_LINHA 0x32

unsigned char mem[NUM_LINHAS][COMPRIMENTO_MAX_LINHA + 1]; // memoria programável
unsigned int regs[NUM_REG];                               // registradores
int estado = INICIO;  // estado da maquina de estados
int charEvento = 0;   // char lido da entrada padrao (stdin)
int tipoDeEvento = 0; // tipo de evento associado ao char (ARROBA, L, X, ...)

int operacao = 0;            // operacao a ser realizada (+,-,*,/)
int acumulador = 0;          //
int registradorAuxiliar = 0; // usado na leitura de numeros do usuario
int operando1 = 0;           // primeiro operando
int ultimoReg = 0; // guarda o ultimo registrador referenciado em algum operando

int linha = 0;  // posicao do cursor
int coluna = 0; // na memoria

/********************************
 *********** REACOES ************
 ********************************/

void printAcc() { printf("> %d\n\n", acumulador); }

// Indica erro de execucao
void reacaoErro() { printf("erro: caractere invalido '%c'\n", charEvento); }

// Atualiza o acumulador com o número fornecido
void reacaoAtualizaAcumulador() {
    acumulador = registradorAuxiliar;
    registradorAuxiliar = 0;
    printAcc();
}

// Armazena o digito fornecido
void reacaoLerDigito() {
    registradorAuxiliar = registradorAuxiliar * 10 + (charEvento - '0');
}

// Armazena o valor do registrador especificado
void reacaoLerRegistrador() {
    ultimoReg = charEvento - '0';
    registradorAuxiliar = regs[ultimoReg];
}

// Armazena a operação fornecida
void reacaoLerOperacao() { operacao = charEvento; }

void reacaoSetOperando1() {
    operando1 = registradorAuxiliar;
    registradorAuxiliar = 0;
}

void reacaoRealizaTilOuX() {
    if (operacao == '~') {
        acumulador = -acumulador;
        printAcc();
    } else if (operacao == 'X') {
        printf("TODO: executar o interpretador");
    }
}

// Armazena a operação fornecida e o numero obtido no acumulador interno
void reacaoLerOperacaoSalvarNum() {
    reacaoLerOperacao();
    operando1 = registradorAuxiliar;
    registradorAuxiliar = 0;
}

// Armazena a operação fornecida e atribui o valor do operando 1
void reacaoLerOperacaoAtualizarOperando() {
    reacaoLerOperacao();
    operando1 = acumulador;
}

void reacaoLerCodigo() {
    mem[linha][coluna] = charEvento;
    coluna++;
}

void reacaoProximaLinha() {
    mem[linha][coluna] = '\0';
    linha += 1;
    coluna = 0;
}

// Armazena o valor do operando 2 e realiza a operação desejada
void reacaoRealizaOperacao() {
    int operando2 = registradorAuxiliar;
    registradorAuxiliar = 0;

    if (operacao == '+') {
        acumulador = operando1 + operando2;
    } else if (operacao == '-') {
        acumulador = operando1 - operando2;
    } else if (operacao == '*') {
        acumulador = operando1 * operando2;
    } else if (operacao == '/') {
        acumulador = operando1 / operando2;
    } else if (operacao == '=') {
        regs[ultimoReg] = acumulador;
    }

    if (operacao == '=') {
        printf("> R%d=%d\n\n", ultimoReg, regs[ultimoReg]);
    } else {
        printAcc();
    }
}

/********************************
 ******* MOTOR DE EVENTOS *******
 ********************************/

// Obtem um evento da entrada padrao (stdin).
// Retorna 1 se foi possivel obter um evento, 0 caso contrario.
void extrairEvento() {
    charEvento = getc(stdin);
    if (('0' <= charEvento && charEvento <= '9')) { // digitos 0-9
        tipoDeEvento = DIGITO;
    } else if (charEvento == '+' || charEvento == '-' || charEvento == '*' ||
               charEvento == '/') {
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
    } else if (charEvento == 'L') {
        tipoDeEvento = L;
    } else if (charEvento == '&') {
        tipoDeEvento = E_COMERCIAL;
    } else {
        tipoDeEvento = OUTROS;
    }
}

int main() {
    while (estado != TERMINAL) {
        extrairEvento();

        switch (estado) {
        case INICIO:
            switch (tipoDeEvento) {
            case DIGITO:
                estado = NUM1;
                reacaoLerDigito();
                break;
            case R:
                estado = REG1;
                break;
            case ENTER:
                // sem efeito
                break;
            case TIL:
            case X:
                estado = ESPERA;
                reacaoLerOperacao();
                break;
            case OP_ARIT:
                estado = NUM2REG2;
                reacaoLerOperacaoAtualizarOperando();
                break;
            case IGUAL:
                estado = SET;
                reacaoLerOperacao();
                break;
            case P:
                estado = BP;
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case NUM1:
            switch (tipoDeEvento) {
            case DIGITO:
                estado = NUM1;
                reacaoLerDigito();
                break;
            case OP_ARIT:
                estado = NUM2REG2;
                reacaoLerOperacaoSalvarNum();
                break;
            case ENTER:
                estado = INICIO;
                reacaoAtualizaAcumulador();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case NUM2:
            switch (tipoDeEvento) {
            case DIGITO:
                estado = NUM2;
                reacaoLerDigito();
                break;
            case ENTER:
                estado = INICIO;
                reacaoRealizaOperacao();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case REG1:
            switch (tipoDeEvento) {
            case DIGITO:
                estado = REG1A;
                reacaoLerRegistrador();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case REG1A:
            switch (tipoDeEvento) {
            case OP_ARIT:
                estado = NUM2REG2;
                reacaoLerOperacaoSalvarNum();
                break;
            case ENTER:
                estado = INICIO;
                reacaoAtualizaAcumulador();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case REG2:
            switch (tipoDeEvento) {
            case DIGITO:
                estado = REG2A;
                reacaoLerRegistrador();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case REG2A:
            switch (tipoDeEvento) {
            case ENTER:
                estado = INICIO;
                reacaoRealizaOperacao();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case NUM2REG2:
            switch (tipoDeEvento) {
            case R:
                estado = REG2;
                break;
            case DIGITO:
                estado = NUM2;
                reacaoLerDigito();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case ESPERA:
            switch (tipoDeEvento) {
            case ENTER:
                estado = INICIO;
                reacaoRealizaTilOuX();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case SET:
            switch (tipoDeEvento) {
            case R:
                estado = REG2;
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case BP:
            switch (tipoDeEvento) {
            case ENTER:
                estado = P_INICIO;
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case PB:
            switch (tipoDeEvento) {
            case ENTER:
                estado = INICIO;
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case P_INICIO:
            switch (tipoDeEvento) {
            case E_COMERCIAL:
                reacaoLerOperacao();
                estado = P_ARG;
                break;
            case L:
                reacaoLerOperacao();
                estado = P_LISTAR;
                break;
            case P:
                estado = PB;
                break;
            case ENTER:
                reacaoProximaLinha();
                break;
            default:
                estado = P_CODIGO;
                reacaoLerCodigo();
                break;
            }
            break;
        case P_CODIGO:
            switch (tipoDeEvento) {
            case ENTER:
                estado = P_INICIO;
                reacaoProximaLinha();
                break;
            default:
                reacaoLerCodigo();
                break;
            }
            break;
        case P_ARG:
            switch (tipoDeEvento) {
            case ENTER:
                estado = P_INICIO;
                reacaoSetOperando1();
                break;
            case DIGITO:
                estado = P_ARG;
                reacaoLerDigito();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case P_LISTAR:
            switch (tipoDeEvento) {
            case ENTER:
                estado = P_INICIO;
                reacaoLerOperacao();
                break;
            case DIGITO:
                estado = P_ARG;
                reacaoLerDigito();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        }
    }
}
