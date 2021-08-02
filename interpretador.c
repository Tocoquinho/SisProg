#include "stdio.h"

// Define um valor numerico para cada um dos cinco estados possiveis
enum Estado {
    INICIO,
    COMENTARIO,
    LABEL_DESCARTE,
    REGDEST,
    REGDESTA,
    SEPARADOR,
    BRANCH,
    LABEL,
    VAL1,
    NUM1,
    REG1,
    REG1A,
    VAL2,
    NUM2,
    REG2,
    REG2A,
    TERMINAL,
};

// Define um valor numerico para cada um dos seis tipos de eventos possiveis
enum TipoDeEvento {
    DIGITO,
    OP_ARIT,
    IGUAL,
    R,
    B,
    VIRGULA,
    ESPACO,
    DOIS_PONTOS,
    PONTO_VIRGULA,
    ENTER,
    PONTO,
    OUTROS,
};

#define NUM_REG 10
#define NUM_LINHAS 0x1024
#define COMPRIMENTO_MAX_LINHA 0x32

unsigned char tipoDeBranch[2];
unsigned char targetLabel[COMPRIMENTO_MAX_LINHA];
int comprimentoTargetLabel = 0;

unsigned char mem[NUM_LINHAS][COMPRIMENTO_MAX_LINHA + 1]; // memoria programável
unsigned int regs[NUM_REG];                               // registradores
int estado = INICIO;  // estado da maquina de estados
int charEvento = 0;   // char lido da entrada padrao (stdin)
int tipoDeEvento = 0; // tipo de evento associado ao char (ARROBA, L, X, ...)

int operacao = 0;            // operacao a ser realizada (+,-,*,/)
int registradorAuxiliar = 0; // usado na leitura de numeros do usuario
int registradorDestino = 0;  // numero do registrador de destino
int operando1 = 0;           // primeiro operando
int linha = 0;
int coluna = 0;

// int linha = 0;  // posicao do cursor
// int coluna = 0; // na memoria

/********************************
 *********** REACOES ************
 ********************************/

void realizarDesvio() {
    for (int i = 0; i < NUM_LINHAS; i++) {
        int sucesso = 1;
        for (int j = 0; targetLabel[j]; j++) {
            if (mem[i][j] != targetLabel[j]) {
                sucesso = 0;
                break;
            }
        }
        if (sucesso) {
            linha = i;
            coluna = 0;
            return;
        }
    }

    printf("Label invalido: ");
    for (int i = 0; i < comprimentoTargetLabel; i++) {
        putc(targetLabel[i], stdout);
    }
}

// Indica erro de execucao
void reacaoErro() { printf("erro: caractere invalido '%c'\n", charEvento); }

// Armazena o digito fornecido
void reacaoLerDigito() {
    registradorAuxiliar = registradorAuxiliar * 10 + (charEvento - '0');
}

// Armazena o valor do registrador especificado
void reacaoLerRegistrador() {
    int ultimoReg = charEvento - '0';
    registradorAuxiliar = regs[ultimoReg];
}

// Armazena a operação fornecida
void reacaoLerOperacao() { operacao = charEvento; }

void reacaoLerTipoDeBranch() {
    tipoDeBranch[0] = tipoDeBranch[1];
    tipoDeBranch[1] = charEvento;
}

void reacaoLerLabel() {
    targetLabel[comprimentoTargetLabel] = charEvento;
    comprimentoTargetLabel++;
}

void reacaoLerRegistradorDestino() { registradorDestino = charEvento - '0'; }

void reacaoSetOperando1() {
    operando1 = registradorAuxiliar;
    registradorAuxiliar = 0;
}

void reacaoDesvioIncondicional() {
    if (tipoDeBranch[0] != 0 || tipoDeBranch[1] != 0) {
        estado = TERMINAL;
        reacaoErro();
        return;
    }

    realizarDesvio();
}

// Armazena o valor do operando 2 e realiza a operação desejada
void reacaoRealizaOperacao() {
    int operando2 = registradorAuxiliar;
    registradorAuxiliar = 0;

    if (operacao == '+') {
        regs[registradorDestino] = operando1 + operando2;
    } else if (operacao == '-') {
        regs[registradorDestino] = operando1 - operando2;
    } else if (operacao == '*') {
        regs[registradorDestino] = operando1 * operando2;
    } else if (operacao == '/') {
        regs[registradorDestino] = operando1 / operando2;
    } else if (operacao == '=') {
        regs[registradorDestino] = operando2;
    } else if (operacao == 'B') {
        int cond;
        if (tipoDeBranch[0] == 'E' && tipoDeBranch[1] == 'Q') {
            cond = operando1 == operando2;
        } else if (tipoDeBranch[0] == 'N' && tipoDeBranch[1] == 'E') {
            cond = operando1 != operando2;
        } else if (tipoDeBranch[0] == 'L' && tipoDeBranch[1] == 'T') {
            cond = operando1 < operando2;
        } else if (tipoDeBranch[0] == 'L' && tipoDeBranch[1] == 'E') {
            cond = operando1 <= operando2;
        } else if (tipoDeBranch[0] == 'G' && tipoDeBranch[1] == 'T') {
            cond = operando1 > operando2;
        } else if (tipoDeBranch[0] == 'G' && tipoDeBranch[1] == 'E') {
            cond = operando1 >= operando2;
        } else {
            estado = TERMINAL;
            reacaoErro();
            return;
        }
        if (cond) {
            realizarDesvio();
        }
    }
}

/********************************
 ******* MOTOR DE EVENTOS *******
 ********************************/

// Obtem um evento da entrada padrao (stdin).
// Retorna 1 se foi possivel obter um evento, 0 caso contrario.
void extrairEvento() {
    charEvento = mem[linha][coluna];
    if (('0' <= charEvento && charEvento <= '9')) { // digitos 0-9
        tipoDeEvento = DIGITO;
    } else if (charEvento == '+' || charEvento == '-' || charEvento == '*' ||
               charEvento == '/') {
        tipoDeEvento = OP_ARIT;
    } else if (charEvento == '=') {
        tipoDeEvento = IGUAL;
    } else if (charEvento == 'R') {
        tipoDeEvento = R;
    } else if (charEvento == 'B') {
        tipoDeEvento = B;
    } else if (charEvento == ',') {
        tipoDeEvento = VIRGULA;
    } else if (charEvento == ' ') {
        tipoDeEvento = ESPACO;
    } else if (charEvento == ':') {
        tipoDeEvento = DOIS_PONTOS;
    } else if (charEvento == ';') {
        tipoDeEvento = PONTO_VIRGULA;
    } else if (charEvento == '\n') {
        tipoDeEvento = ENTER;
    } else if (charEvento == '.') {
        tipoDeEvento = PONTO;
    } else {
        tipoDeEvento = OUTROS;
    }
}

int main() {
    while (estado != TERMINAL) {
        extrairEvento();

        switch (estado) {
        case INICIO:
            switch (charEvento) {
            case PONTO_VIRGULA:
                estado = COMENTARIO;
                break;
            case DOIS_PONTOS:
                estado = LABEL_DESCARTE;
                break;
            case OP_ARIT:
            case IGUAL:
                estado = REGDEST;
                reacaoLerOperacao();
                break;
            case B:
                estado = BRANCH;
                reacaoLerOperacao();
                break;
            case ESPACO:
            case ENTER:
                // sem efeito
                break;
            case PONTO:
                estado = TERMINAL;
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case COMENTARIO:
            switch (charEvento) {
            case ENTER:
                estado = INICIO;
                break;
            default:
                // sem efeito
                break;
            }
            break;
        case LABEL_DESCARTE:
            switch (charEvento) {
            case ESPACO:
                estado = INICIO;
                break;
            case VIRGULA:
                estado = TERMINAL;
                reacaoErro();
                break;
            default:
                // sem efeito
                break;
            }
            break;
        case REGDEST:
            switch (charEvento) {
            case R:
                estado = REGDESTA;
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case REGDESTA:
            switch (charEvento) {
            case DIGITO:
                estado = SEPARADOR;
                reacaoLerRegistradorDestino();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case SEPARADOR:
            switch (charEvento) {
            case VIRGULA:
                estado = (operacao == '=') ? VAL2 : VAL1;
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case BRANCH:
            switch (charEvento) {
            case ESPACO:
                estado = LABEL;
                break;
            default:
                estado = BRANCH;
                reacaoLerTipoDeBranch();
                break;
            }
            break;
        case LABEL:
            switch (charEvento) {
            case VIRGULA:
                estado = VAL1;
                break;
            case ESPACO:
                estado = TERMINAL;
                reacaoErro();
                break;
            case ENTER:
                estado = INICIO;
                reacaoDesvioIncondicional();
                break;
            default:
                reacaoLerLabel();
                break;
            }
            break;
        case VAL1:
            switch (charEvento) {
            case DIGITO:
                estado = NUM1;
                reacaoLerDigito();
                break;
            case R:
                estado = REG1;
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case NUM1:
            switch (charEvento) {
            case DIGITO:
                reacaoLerDigito();
                break;
            case VIRGULA:
                estado = VAL2;
                reacaoSetOperando1();
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case REG1:
            switch (charEvento) {
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
            switch (charEvento) {
            case VIRGULA:
                estado = VAL2;
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case VAL2:
            switch (charEvento) {
            case DIGITO:
                estado = NUM2;
                reacaoLerDigito();
                break;
            case R:
                estado = REG2;
                break;
            default:
                estado = TERMINAL;
                reacaoErro();
                break;
            }
            break;
        case NUM2:
            switch (charEvento) {
            case DIGITO:
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
        case REG2:
            switch (charEvento) {
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
            switch (charEvento) {
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
        }

        coluna++;
        if (mem[linha][coluna] == 0) {
            linha++;
            coluna = 0;
        }
    }
}
