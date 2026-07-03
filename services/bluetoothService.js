// services/bluetoothService.js
import RNBluetoothClassic from 'react-native-bluetooth-classic';

let dispositivoConectado = null;
let assinaturaLeitura = null;

const HC05_MAC_ADDRESS = 'HC05';

export const BluetoothService = {
    // Função para conectar
    conectar: async () => {
        try {
            if (dispositivoConectado) return dispositivoConectado; // Já está conectado

            const conexao = await RNBluetoothClassic.connectToDevice(HC05_MAC_ADDRESS);
            dispositivoConectado = conexao;
            return conexao;
        } catch (error) {
            console.error("Erro ao conectar no serviço:", error);
            throw error;
        }
    },

    // Função para desconectar
    desconectar: async () => {
        if (assinaturaLeitura) {
            assinaturaLeitura.remove(); // Para de escutar a porta serial
            assinaturaLeitura = null;
        }
        if (dispositivoConectado) {
            await dispositivoConectado.disconnect();
            dispositivoConectado = null;
        }
    },

    // Função para enviar dados (comandos '1', '0', 'A')
    enviarComando: async (comando) => {
        if (dispositivoConectado) {
            try {
                await dispositivoConectado.write(comando);
            } catch (error) {
                console.error("Erro ao enviar comando:", error);
            }
        } else {
            console.warn("Nenhum dispositivo conectado para enviar comando.");
        }
    },

    // Função para escutar os dados recebidos do Arduino
    escutarDados: (onDataReceivedCallback) => {
        if (!dispositivoConectado) {
            console.warn("Conecte primeiro antes de escutar os dados.");
            return;
        }

        // Remove assinatura anterior se existir para não duplicar listeners
        if (assinaturaLeitura) assinaturaLeitura.remove();

        // Inscreve o callback que vai atualizar os estados lá na sua tela
        assinaturaLeitura = dispositivoConectado.onDataReceived((event) => {
            cconst[tipoValor, estado] = dadosRaw.split(',');

            if (tipoValor.startsWith('U:')) {
                onDataReceivedCallback({
                    tipo: 'umidade',
                    umidade: tipoValor.replace('U:', ''),
                    estadoUmidade: estado.trim()
                });
            }

            if (tipoValor.startsWith('T:')) {
                onDataReceivedCallback({
                    tipo: 'temperatura',
                    temperatura: tipoValor.replace('T:', ''),
                    estadoTemperatura: estado.trim()
                });
            }
        });
    },

    pararEscuta: () => {
        if (assinaturaLeitura) {
            assinaturaLeitura.remove();
            assinaturaLeitura = null;
            console.log("Escuta de dados serial finalizada, mas Bluetooth continua conectado.");
        }
    },
};
