#include "stdio.h"

// numero de registradores
#define NUM_REG 10

// numero de linhas programaveis
#define NUM_LINHAS 1000

// comprimento maximo de uma linha programavel
#define COMPRIMENTO_MAX_LINHA 32

// Define um valor numerico para cada um dos estados possiveis para a
// calculadora
enum Calc_Estado {
    CALC_INICIO,
    CALC_NUM1,
    CALC_NUM2,
    CALC_REG1,
    CALC_REG1A,
    CALC_REG2,
    CALC_REG2A,
    CALC_NUM2REG2,
    CALC_ESPERA,
    CALC_SET,
    CALC_BP,
    CALC_PB,
    CALC_P_INICIO,
    CALC_P_CODIGO,
    CALC_P_ARG,
    CALC_P_LISTAR,
    CALC_TERMINAL,
};

// Define um valor numerico para cada um dos tipos de eventos possiveis para a
// calculadora
enum Calc_TipoDeEvento {
    CALC_DIGITO,
    CALC_OP_ARIT,
    CALC_IGUAL,
    CALC_R,
    CALC_TIL,
    CALC_C,
    CALC_S,
    CALC_ENTER,
    CALC_P,
    CALC_L,
    CALC_E_COMERCIAL,
    CALC_OUTROS,
};

void interp_rodarMotorDeEventos();
int interp_ehModoContinuo;

unsigned char programa[NUM_LINHAS]
                      [COMPRIMENTO_MAX_LINHA + 2]; // memoria programável
unsigned int regs[NUM_REG];                        // registradores

int calc_estado = CALC_INICIO; // estado da maquina de estados
int calc_charEvento = 0;       // char lido da entrada padrao (stdin)
int calc_tipoDeEvento = 0;     // tipo do evento (Calc_TIPO_DE_EVENTO)

int calc_operacao = 0;            // operacao a ser realizada (+,-,*,/)
int calc_acumulador = 0;          // acumulador da calculadora
int calc_registradorAuxiliar = 0; // usado na leitura de numeros do usuario
int calc_operando1 = 0;           // primeiro operando em operacoes binarias

// guarda o ultimo registrador que foi referenciado em algum operando
int calc_ultimoReg = 0;

int calc_linha = 0;  // posicao do cursor
int calc_coluna = 0; // na memoria

int calc_maiorLinha = 0; // maior numero de linha que ja foi escrito

/********************************
 *********** REACOES ************
 ********************************/

void calc_printAcc() { printf("> %d\n", calc_acumulador); }

void calc_listar(int linha) {
    if (programa[linha][0] == 0) {
        printf("[%03d] \n", linha);
    } else {
        printf("[%03d] %s", linha, programa[linha]);
    }
}

// Indica erro de execucao
void calc_reacaoErro() {
    printf("Erro: caractere invalido '%c'\n", calc_charEvento);
}

// Atualiza o acumulador com o número fornecido
void reacaoAtualizaAcumulador() {
    calc_acumulador = calc_registradorAuxiliar;
    calc_registradorAuxiliar = 0;
    calc_printAcc();
}

// Armazena o digito fornecido
void calc_reacaoLerDigito() {
    calc_registradorAuxiliar =
        calc_registradorAuxiliar * 10 + (calc_charEvento - '0');
}

// Armazena o valor do registrador especificado
void calc_reacaoLerRegistrador() {
    calc_ultimoReg = calc_charEvento - '0';
    calc_registradorAuxiliar = regs[calc_ultimoReg];
}

void calc_reacaoListarOuAlterarLinha() {
    if (calc_operacao == 'L') {
        calc_listar(calc_registradorAuxiliar);
    } else if (calc_operacao == '&') {
        calc_linha = calc_registradorAuxiliar;
        calc_coluna = 0;
    }
    calc_registradorAuxiliar = 0;
}

void calc_reacaoListarTudo() {
    for (int i = 0; i <= calc_maiorLinha; i++) {
        calc_listar(i);
    }
}

// Armazena a operação fornecida
void calc_reacaoLerOperacao() { calc_operacao = calc_charEvento; }

void calc_reacaoSetOperando1() {
    calc_operando1 = calc_registradorAuxiliar;
    calc_registradorAuxiliar = 0;
}

void calc_reacaoRealizaTilOuX() {
    if (calc_operacao == '~') {
        calc_acumulador = -calc_acumulador;
        calc_printAcc();
    } else if (calc_operacao == 'C') {
        interp_ehModoContinuo = 1;
        interp_rodarMotorDeEventos();
    } else if (calc_operacao == 'S') {
        interp_ehModoContinuo = 0;
        interp_rodarMotorDeEventos();
    }
}

// Armazena a operação fornecida e o numero obtido no acumulador interno
void calc_reacaoLerOperacaoSalvarNum() {
    calc_reacaoLerOperacao();
    calc_operando1 = calc_registradorAuxiliar;
    calc_registradorAuxiliar = 0;
}

// Armazena a operação fornecida e atribui o valor do operando 1
void calc_reacaoLerOperacaoAtualizarOperando() {
    calc_reacaoLerOperacao();
    calc_operando1 = calc_acumulador;
}

void calc_reacaoLerCodigo() {
    if (calc_coluna >= COMPRIMENTO_MAX_LINHA) {
        if (calc_coluna == COMPRIMENTO_MAX_LINHA) {
            printf("Erro: linha grande demais\n");
        }
        return;
    }
    programa[calc_linha][calc_coluna] = calc_charEvento;
    calc_coluna++;
}

void calc_reacaoProximaLinha() {
    programa[calc_linha][calc_coluna] = '\n';
    programa[calc_linha][calc_coluna + 1] = '\0';
    calc_linha += 1;
    if (calc_linha > calc_maiorLinha) {
        calc_maiorLinha = calc_linha;
    }
    calc_coluna = 0;
}

// Armazena o valor do operando 2 e realiza a operação desejada
void calc_reacaoRealizaOperacao() {
    int operando2 = calc_registradorAuxiliar;
    calc_registradorAuxiliar = 0;

    if (calc_operacao == '+') {
        calc_acumulador = calc_operando1 + operando2;
    } else if (calc_operacao == '-') {
        calc_acumulador = calc_operando1 - operando2;
    } else if (calc_operacao == '*') {
        calc_acumulador = calc_operando1 * operando2;
    } else if (calc_operacao == '/') {
        calc_acumulador = calc_operando1 / operando2;
    } else if (calc_operacao == '=') {
        regs[calc_ultimoReg] = calc_acumulador;
    }

    if (calc_operacao == '=') {
        printf("> R%d=%d\n", calc_ultimoReg, regs[calc_ultimoReg]);
    } else {
        calc_printAcc();
    }
}

/********************************
 ******* MOTOR DE EVENTOS *******
 ********************************/

// Obtem um evento da entrada padrao (stdin).
void calc_extrairEvento() {
    calc_charEvento = getc(stdin);
    if (('0' <= calc_charEvento && calc_charEvento <= '9')) { // digitos 0-9
        calc_tipoDeEvento = CALC_DIGITO;
    } else if (calc_charEvento == '+' || calc_charEvento == '-' ||
               calc_charEvento == '*' || calc_charEvento == '/') {
        calc_tipoDeEvento = CALC_OP_ARIT;
    } else if (calc_charEvento == '=') {
        calc_tipoDeEvento = CALC_IGUAL;
    } else if (calc_charEvento == 'R') {
        calc_tipoDeEvento = CALC_R;
    } else if (calc_charEvento == '~') {
        calc_tipoDeEvento = CALC_TIL;
    } else if (calc_charEvento == 'C') {
        calc_tipoDeEvento = CALC_C;
    } else if (calc_charEvento == 'S') {
        calc_tipoDeEvento = CALC_S;
    } else if (calc_charEvento == '\n') {
        calc_tipoDeEvento = CALC_ENTER;
    } else if (calc_charEvento == 'P') {
        calc_tipoDeEvento = CALC_P;
    } else if (calc_charEvento == 'L') {
        calc_tipoDeEvento = CALC_L;
    } else if (calc_charEvento == '&') {
        calc_tipoDeEvento = CALC_E_COMERCIAL;
    } else {
        calc_tipoDeEvento = CALC_OUTROS;
    }
}

// Roda o loop principal do motor de eventos da calculadora
void calc_rodarMotorDeEventos() {
    // Reseta os valores das variaveis globais
    // (ver descricao no topo do arquivo)
    calc_estado = CALC_INICIO;
    calc_charEvento = 0;
    calc_tipoDeEvento = 0;
    calc_operacao = 0;
    calc_acumulador = 0;
    calc_registradorAuxiliar = 0;
    calc_operando1 = 0;
    calc_ultimoReg = 0;
    calc_linha = 0;
    calc_coluna = 0;
    calc_maiorLinha = 0;

    while (calc_estado != CALC_TERMINAL) {
        if (calc_estado == CALC_INICIO) {
            printf("calc$ ");
        } else if (calc_estado == CALC_P_INICIO) {
            printf("[%03d] prog$ ", calc_linha);
        }
        calc_extrairEvento();

        switch (calc_estado) {
        case CALC_INICIO:
            switch (calc_tipoDeEvento) {
            case CALC_DIGITO:
                calc_estado = CALC_NUM1;
                calc_reacaoLerDigito();
                break;
            case CALC_R:
                calc_estado = CALC_REG1;
                break;
            case CALC_ENTER:
                // sem efeito
                break;
            case CALC_TIL:
            case CALC_C:
            case CALC_S:
                calc_estado = CALC_ESPERA;
                calc_reacaoLerOperacao();
                break;
            case CALC_OP_ARIT:
                calc_estado = CALC_NUM2REG2;
                calc_reacaoLerOperacaoAtualizarOperando();
                break;
            case CALC_IGUAL:
                calc_estado = CALC_SET;
                calc_reacaoLerOperacao();
                break;
            case CALC_P:
                calc_estado = CALC_BP;
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_NUM1:
            switch (calc_tipoDeEvento) {
            case CALC_DIGITO:
                calc_estado = CALC_NUM1;
                calc_reacaoLerDigito();
                break;
            case CALC_OP_ARIT:
                calc_estado = CALC_NUM2REG2;
                calc_reacaoLerOperacaoSalvarNum();
                break;
            case CALC_ENTER:
                calc_estado = CALC_INICIO;
                reacaoAtualizaAcumulador();
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_NUM2:
            switch (calc_tipoDeEvento) {
            case CALC_DIGITO:
                calc_estado = CALC_NUM2;
                calc_reacaoLerDigito();
                break;
            case CALC_ENTER:
                calc_estado = CALC_INICIO;
                calc_reacaoRealizaOperacao();
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_REG1:
            switch (calc_tipoDeEvento) {
            case CALC_DIGITO:
                calc_estado = CALC_REG1A;
                calc_reacaoLerRegistrador();
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_REG1A:
            switch (calc_tipoDeEvento) {
            case CALC_OP_ARIT:
                calc_estado = CALC_NUM2REG2;
                calc_reacaoLerOperacaoSalvarNum();
                break;
            case CALC_ENTER:
                calc_estado = CALC_INICIO;
                reacaoAtualizaAcumulador();
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_REG2:
            switch (calc_tipoDeEvento) {
            case CALC_DIGITO:
                calc_estado = CALC_REG2A;
                calc_reacaoLerRegistrador();
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_REG2A:
            switch (calc_tipoDeEvento) {
            case CALC_ENTER:
                calc_estado = CALC_INICIO;
                calc_reacaoRealizaOperacao();
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_NUM2REG2:
            switch (calc_tipoDeEvento) {
            case CALC_R:
                calc_estado = CALC_REG2;
                break;
            case CALC_DIGITO:
                calc_estado = CALC_NUM2;
                calc_reacaoLerDigito();
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_ESPERA:
            switch (calc_tipoDeEvento) {
            case CALC_ENTER:
                calc_estado = CALC_INICIO;
                calc_reacaoRealizaTilOuX();
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_SET:
            switch (calc_tipoDeEvento) {
            case CALC_R:
                calc_estado = CALC_REG2;
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_BP:
            switch (calc_tipoDeEvento) {
            case CALC_ENTER:
                calc_estado = CALC_P_INICIO;
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_PB:
            switch (calc_tipoDeEvento) {
            case CALC_ENTER:
                calc_estado = CALC_INICIO;
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_P_INICIO:
            switch (calc_tipoDeEvento) {
            case CALC_E_COMERCIAL:
                calc_reacaoLerOperacao();
                calc_estado = CALC_P_ARG;
                break;
            case CALC_L:
                calc_reacaoLerOperacao();
                calc_estado = CALC_P_LISTAR;
                break;
            case CALC_P:
                calc_estado = CALC_PB;
                break;
            case CALC_ENTER:
                calc_reacaoProximaLinha();
                break;
            default:
                calc_estado = CALC_P_CODIGO;
                calc_reacaoLerCodigo();
                break;
            }
            break;
        case CALC_P_CODIGO:
            switch (calc_tipoDeEvento) {
            case CALC_ENTER:
                calc_estado = CALC_P_INICIO;
                calc_reacaoProximaLinha();
                break;
            default:
                calc_reacaoLerCodigo();
                break;
            }
            break;
        case CALC_P_ARG:
            switch (calc_tipoDeEvento) {
            case CALC_ENTER:
                calc_estado = CALC_P_INICIO;
                calc_reacaoListarOuAlterarLinha();
                break;
            case CALC_DIGITO:
                calc_estado = CALC_P_ARG;
                calc_reacaoLerDigito();
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        case CALC_P_LISTAR:
            switch (calc_tipoDeEvento) {
            case CALC_ENTER:
                calc_estado = CALC_P_INICIO;
                calc_reacaoListarTudo();
                break;
            case CALC_DIGITO:
                calc_estado = CALC_P_ARG;
                calc_reacaoLerDigito();
                break;
            default:
                calc_estado = CALC_TERMINAL;
                calc_reacaoErro();
                break;
            }
            break;
        }
    }
}
