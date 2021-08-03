#include "stdio.h"

// Define um valor numerico para cada um dos cinco estados possiveis
enum Interp_Estado {
    INTERP_INICIO,
    INTERP_AGUARDA_PASSO,
    INTERP_COMENTARIO,
    INTERP_LABEL_DESCARTE,
    INTERP_REGDEST,
    INTERP_REGDESTA,
    INTERP_SEPARADOR,
    INTERP_BRANCH,
    INTERP_LABEL,
    INTERP_VAL1,
    INTERP_NUM1,
    INTERP_REG1,
    INTERP_REG1A,
    INTERP_VAL2,
    INTERP_NUM2,
    INTERP_REG2,
    INTERP_REG2A,
    INTERP_FIM,
    INTERP_TERMINAL,
};

// Define um valor numerico para cada um dos seis tipos de eventos possiveis
enum Interp_TipoDeEvento {
    INTERP_DIGITO,
    INTERP_OP_ARIT,
    INTERP_IGUAL,
    INTERP_R,
    INTERP_B,
    INTERP_VIRGULA,
    INTERP_ESPACO,
    INTERP_DOIS_PONTOS,
    INTERP_PONTO_VIRGULA,
    INTERP_ENTER,
    INTERP_PONTO,
    INTERP_OUTROS,
};

// indica se o interpolador esta executando em modo continuo
int interp_ehModoContinuo = 0;

unsigned char interp_tipoDeBranch[2];
unsigned char interp_targetLabel[COMPRIMENTO_MAX_LINHA];
int interp_comprimentoTargetLabel = 0;

int interp_estado = INTERP_INICIO; // estado da maquina de estados
int interp_charEvento = 0;         // char lido do codigo programado
int interp_tipoDeEvento = 0;       // tipo do evento (Interp_TIPO_DE_EVENTO)

int interp_operacao = 0;            // operacao a ser realizada (+,-,*,/)
int interp_registradorAuxiliar = 0; // usado na leitura de numeros do usuario
int interp_registradorDestino = 0;  // numero do registrador de destino
int interp_operando1 = 0;           // primeiro operando
int interp_linha = 0;
int interp_coluna = 0;

/********************************
 *********** REACOES ************
 ********************************/

void interp_realizarDesvio() {
    for (int i = 0; i < NUM_LINHAS; i++) {
        int sucesso = 1;
        for (int j = 0; j < interp_comprimentoTargetLabel; j++) {
            if (programa[i][j + 1] != interp_targetLabel[j]) {
                sucesso = 0;
                break;
            }
        }
        if (sucesso && programa[i][0] == ':') {
            interp_linha = i;
            interp_coluna = -1; // esse valor sera incrementado pelo motor de
                                // eventos antes da proxima iteracao
            interp_estado =
                interp_ehModoContinuo ? INTERP_INICIO : INTERP_AGUARDA_PASSO;
            interp_comprimentoTargetLabel = 0; // reseta target

            if (!interp_ehModoContinuo) {
                printf("Desvio para a linha %d\n", interp_linha);
            }
            return;
        }
    }

    printf("Erro: label invalido (linha %d): ", interp_linha);
    for (int i = 0; i < interp_comprimentoTargetLabel; i++) {
        putc(interp_targetLabel[i], stdout);
    }
    printf("\n");
    interp_estado = INTERP_TERMINAL;
    interp_comprimentoTargetLabel = 0; // reseta target
}

// Indica erro de execucao
void interp_reacaoErro() {
    printf("Erro: caractere invalido '%c' na linha %d, coluna %d em:\n",
           interp_charEvento, interp_linha, interp_coluna);
    if (interp_linha > 0)
        calc_listar(interp_linha - 1);
    calc_listar(interp_linha);
    if (interp_linha < NUM_LINHAS - 1)
        calc_listar(interp_linha + 1);
    printf("\n");
}

// Armazena o digito fornecido
void interp_reacaoLerDigito() {
    interp_registradorAuxiliar =
        interp_registradorAuxiliar * 10 + (interp_charEvento - '0');
}

// Armazena o valor do registrador especificado
void interp_reacaoLerRegistrador() {
    int ultimoReg = interp_charEvento - '0';
    interp_registradorAuxiliar = regs[ultimoReg];
}

// Armazena a operação fornecida
void interp_reacaoLerOperacao() { interp_operacao = interp_charEvento; }

void interp_reacaoLerTipoDeBranch() {
    interp_tipoDeBranch[0] = interp_tipoDeBranch[1];
    interp_tipoDeBranch[1] = interp_charEvento;
}

void interp_reacaoLerLabel() {
    interp_targetLabel[interp_comprimentoTargetLabel] = interp_charEvento;
    interp_comprimentoTargetLabel++;
}

void interp_reacaoLerRegistradorDestino() {
    interp_registradorDestino = interp_charEvento - '0';
}

void interp_reacaoSetOperando1() {
    interp_operando1 = interp_registradorAuxiliar;
    interp_registradorAuxiliar = 0;
}

void interp_reacaoDesvioIncondicional() {
    if (interp_tipoDeBranch[0] != 0 || interp_tipoDeBranch[1] != 0) {
        interp_estado = INTERP_TERMINAL;
        interp_reacaoErro();
        return;
    }

    interp_realizarDesvio();
}

// Armazena o valor do operando 2 e realiza a operação desejada
void interp_reacaoRealizaOperacao() {
    int operando2 = interp_registradorAuxiliar;
    interp_registradorAuxiliar = 0;

    if (interp_operacao == '+') {
        regs[interp_registradorDestino] = interp_operando1 + operando2;
    } else if (interp_operacao == '-') {
        regs[interp_registradorDestino] = interp_operando1 - operando2;
    } else if (interp_operacao == '*') {
        regs[interp_registradorDestino] = interp_operando1 * operando2;
    } else if (interp_operacao == '/') {
        regs[interp_registradorDestino] = interp_operando1 / operando2;
    } else if (interp_operacao == '=') {
        regs[interp_registradorDestino] = operando2;
    } else if (interp_operacao == 'B') {
        int cond;
        if (interp_tipoDeBranch[0] == 'E' && interp_tipoDeBranch[1] == 'Q') {
            cond = interp_operando1 == operando2;
        } else if (interp_tipoDeBranch[0] == 'N' &&
                   interp_tipoDeBranch[1] == 'E') {
            cond = interp_operando1 != operando2;
        } else if (interp_tipoDeBranch[0] == 'L' &&
                   interp_tipoDeBranch[1] == 'T') {
            cond = interp_operando1 < operando2;
        } else if (interp_tipoDeBranch[0] == 'L' &&
                   interp_tipoDeBranch[1] == 'E') {
            cond = interp_operando1 <= operando2;
        } else if (interp_tipoDeBranch[0] == 'G' &&
                   interp_tipoDeBranch[1] == 'T') {
            cond = interp_operando1 > operando2;
        } else if (interp_tipoDeBranch[0] == 'G' &&
                   interp_tipoDeBranch[1] == 'E') {
            cond = interp_operando1 >= operando2;
        } else {
            interp_estado = INTERP_TERMINAL;
            interp_reacaoErro();
            return;
        }
        if (cond) {
            interp_realizarDesvio();
        }
        interp_comprimentoTargetLabel = 0;
    }
}

/********************************
 ******* MOTOR DE EVENTOS *******
 ********************************/

// Obtem um evento do codigo programado (variavel `programa`)
void interp_extrairEvento() {
    if (interp_estado == INTERP_AGUARDA_PASSO) {
        interp_charEvento = getc(stdin);
    } else {
        interp_charEvento = programa[interp_linha][interp_coluna];
    }
    if (('0' <= interp_charEvento && interp_charEvento <= '9')) { // digitos 0-9
        interp_tipoDeEvento = INTERP_DIGITO;
    } else if (interp_charEvento == '+' || interp_charEvento == '-' ||
               interp_charEvento == '*' || interp_charEvento == '/') {
        interp_tipoDeEvento = INTERP_OP_ARIT;
    } else if (interp_charEvento == '=') {
        interp_tipoDeEvento = INTERP_IGUAL;
    } else if (interp_charEvento == 'R') {
        interp_tipoDeEvento = INTERP_R;
    } else if (interp_charEvento == 'B') {
        interp_tipoDeEvento = INTERP_B;
    } else if (interp_charEvento == ',') {
        interp_tipoDeEvento = INTERP_VIRGULA;
    } else if (interp_charEvento == ' ') {
        interp_tipoDeEvento = INTERP_ESPACO;
    } else if (interp_charEvento == ':') {
        interp_tipoDeEvento = INTERP_DOIS_PONTOS;
    } else if (interp_charEvento == ';') {
        interp_tipoDeEvento = INTERP_PONTO_VIRGULA;
    } else if (interp_charEvento == '\n') {
        interp_tipoDeEvento = INTERP_ENTER;
    } else if (interp_charEvento == '.') {
        interp_tipoDeEvento = INTERP_PONTO;
    } else {
        interp_tipoDeEvento = INTERP_OUTROS;
    }
}

// Roda o loop principal do motor de eventos do interpretador
void interp_rodarMotorDeEventos() {
    // Reseta os valores das variaveis globais
    // (ver descricao no topo do arquivo)
    interp_estado =
        interp_ehModoContinuo ? INTERP_INICIO : INTERP_AGUARDA_PASSO;
    interp_charEvento = 0;
    interp_tipoDeEvento = 0;
    interp_operacao = 0;
    interp_registradorAuxiliar = 0;
    interp_registradorDestino = 0;
    interp_operando1 = 0;
    interp_linha = 0;
    interp_coluna = 0;
    interp_comprimentoTargetLabel = 0;

    int ultimaLinhaImpressa = -1;

    while (interp_estado != INTERP_TERMINAL) {
        if (!interp_ehModoContinuo && ultimaLinhaImpressa != interp_linha) {
            for (int i = 0; i < NUM_REG; i++) {
                printf("R%d=%d ", i, regs[i]);
            }
            printf("\nProxima linha:  ");
            calc_listar(interp_linha);

            ultimaLinhaImpressa = interp_linha;
            printf("Auxiliar:  %d\n", interp_registradorAuxiliar);
        }

        interp_extrairEvento();

        switch (interp_estado) {
        case INTERP_INICIO:
            switch (interp_tipoDeEvento) {
            case INTERP_PONTO_VIRGULA:
                interp_estado = INTERP_COMENTARIO;
                break;
            case INTERP_DOIS_PONTOS:
                interp_estado = INTERP_LABEL_DESCARTE;
                interp_reacaoLerOperacao();
                break;
            case INTERP_OP_ARIT:
            case INTERP_IGUAL:
                interp_estado = INTERP_REGDEST;
                interp_reacaoLerOperacao();
                break;
            case INTERP_B:
                interp_estado = INTERP_BRANCH;
                interp_reacaoLerOperacao();
                break;
            case INTERP_ESPACO:
                // sem efeitos
                break;
            case INTERP_ENTER:
                interp_estado = interp_ehModoContinuo ? INTERP_INICIO
                                                      : INTERP_AGUARDA_PASSO;
                break;
            case INTERP_PONTO:
                interp_estado = INTERP_FIM;
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_AGUARDA_PASSO:
            switch (interp_tipoDeEvento) {
            case INTERP_ENTER:
                interp_estado = INTERP_INICIO;
                break;
            default:
                // sem efeito
                break;
            }
            continue;
        case INTERP_COMENTARIO:
            switch (interp_tipoDeEvento) {
            case INTERP_ENTER:
                interp_estado = interp_ehModoContinuo ? INTERP_INICIO
                                                      : INTERP_AGUARDA_PASSO;
                break;
            default:
                // sem efeito
                break;
            }
            break;
        case INTERP_LABEL_DESCARTE:
            switch (interp_tipoDeEvento) {
            case INTERP_ESPACO:
                interp_estado = INTERP_INICIO;
                break;
            case INTERP_VIRGULA:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            default:
                // sem efeito
                break;
            }
            break;
        case INTERP_REGDEST:
            switch (interp_tipoDeEvento) {
            case INTERP_R:
                interp_estado = INTERP_REGDESTA;
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_REGDESTA:
            switch (interp_tipoDeEvento) {
            case INTERP_DIGITO:
                interp_estado = INTERP_SEPARADOR;
                interp_reacaoLerRegistradorDestino();
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_SEPARADOR:
            switch (interp_tipoDeEvento) {
            case INTERP_VIRGULA:
                interp_estado =
                    (interp_operacao == '=') ? INTERP_VAL2 : INTERP_VAL1;
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_BRANCH:
            switch (interp_tipoDeEvento) {
            case INTERP_ESPACO:
                interp_estado = INTERP_LABEL;
                break;
            default:
                interp_estado = INTERP_BRANCH;
                interp_reacaoLerTipoDeBranch();
                break;
            }
            break;
        case INTERP_LABEL:
            switch (interp_tipoDeEvento) {
            case INTERP_VIRGULA:
                interp_estado = INTERP_VAL1;
                break;
            case INTERP_ESPACO:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            case INTERP_ENTER:
                interp_estado = interp_ehModoContinuo ? INTERP_INICIO
                                                      : INTERP_AGUARDA_PASSO;
                interp_reacaoDesvioIncondicional();
                break;
            default:
                interp_reacaoLerLabel();
                break;
            }
            break;
        case INTERP_VAL1:
            switch (interp_tipoDeEvento) {
            case INTERP_DIGITO:
                interp_estado = INTERP_NUM1;
                interp_reacaoLerDigito();
                break;
            case INTERP_R:
                interp_estado = INTERP_REG1;
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_NUM1:
            switch (interp_tipoDeEvento) {
            case INTERP_DIGITO:
                interp_reacaoLerDigito();
                break;
            case INTERP_VIRGULA:
                interp_estado = INTERP_VAL2;
                interp_reacaoSetOperando1();
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_REG1:
            switch (interp_tipoDeEvento) {
            case INTERP_DIGITO:
                interp_estado = INTERP_REG1A;
                interp_reacaoLerRegistrador();
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_REG1A:
            switch (interp_tipoDeEvento) {
            case INTERP_VIRGULA:
                interp_estado = INTERP_VAL2;
                interp_reacaoSetOperando1();
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_VAL2:
            switch (interp_tipoDeEvento) {
            case INTERP_DIGITO:
                interp_estado = INTERP_NUM2;
                interp_reacaoLerDigito();
                break;
            case INTERP_R:
                interp_estado = INTERP_REG2;
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_NUM2:
            switch (interp_tipoDeEvento) {
            case INTERP_DIGITO:
                interp_reacaoLerDigito();
                break;
            case INTERP_ENTER:
                interp_estado = interp_ehModoContinuo ? INTERP_INICIO
                                                      : INTERP_AGUARDA_PASSO;
                interp_reacaoRealizaOperacao();
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_REG2:
            switch (interp_tipoDeEvento) {
            case INTERP_DIGITO:
                interp_estado = INTERP_REG2A;
                interp_reacaoLerRegistrador();
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_REG2A:
            switch (interp_tipoDeEvento) {
            case INTERP_ENTER:
                interp_estado = interp_ehModoContinuo ? INTERP_INICIO
                                                      : INTERP_AGUARDA_PASSO;
                interp_reacaoRealizaOperacao();
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        case INTERP_FIM:
            switch (interp_tipoDeEvento) {
            case INTERP_ENTER:
                interp_estado = INTERP_TERMINAL;
                break;
            default:
                interp_estado = INTERP_TERMINAL;
                interp_reacaoErro();
                break;
            }
            break;
        }

        interp_coluna++;
        if (programa[interp_linha][interp_coluna] == 0) {
            interp_linha++;
            interp_coluna = 0;
        }
        // printf("linha=%d coluna=%d estado=%d\n", interp_linha,
        //        interp_coluna, interp_estado);
        if (interp_linha >= NUM_LINHAS) {
            interp_estado = INTERP_TERMINAL;
            printf("Erro: fim do programa atingido sem comando de fim "
                   "('.')\n");
        }
    }
}
