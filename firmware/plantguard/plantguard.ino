import { StatusBar } from 'expo-status-bar';
import { StyleSheet, Text, View } from 'react-native';
import React, { useState, useEffect } from 'react';
import { BluetoothService } from './services/bluetoothService';
import { TouchableOpacity } from 'react-native';

export default function Monitoramento() {

    // Dados que vêm do Arduino
    const [umidade, setUmidade] = useState('--');
    const [estadoUmidade, setEstadoUmidade] = useState('--');
    const [temperatura, setTemperatura] = useState('--');
    const [estadoTemperatura, setEstadoTemperatura] = useState('--');

    // Modos de operação
    const [modoAutomatico, setModoAutomatico] = useState(true);
    const [irrigando, setIrrigando] = useState(false);


    useEffect(() => {
        const iniciarSistema = async () => {
            try {
                await BluetoothService.conectar();
                Alert.alert("Sucesso", "Conectado ao Plant Guard!");

                // Começa a escutar o Arduino e passa o que fazer com os dados
                BluetoothService.escutarDados((dadosRecortados) => {
                    switch (dadosRecortados.tipo) {

                        case 'umidade':
                            setUmidade(dadosRecortados.umidade);
                            setEstadoUmidade(dadosRecortados.estadoUmidade);
                            break;

                        case 'temperatura':
                            setTemperatura(dadosRecortados.temperatura);
                            setEstadoTemperatura(dadosRecortados.estadoTemperatura);
                            break;

                        default:
                            console.log("Tipo desconhecido:", dadosRecortados);
                    }
                });

            } catch (error) {
                Alert.alert("Erro", "Não foi possível conectar ao dispositivo.");
            }
        };

        return () => {
            // Desliga a escuta desta tela específica
            BluetoothService.pararEscuta();
        };
    }, []);

    /* Ao irrigar ou desligar "manualmente" o modo automático é desativado
    */
    const irrigacao = () => {
        if (!irrigando) {
            setIrrigando(true);
            setModoAutomatico(false);
            BluetoothService.enviarComando('1');
        } else {
            setIrrigando(false);
            setModoAutomatico(false);
            BluetoothService.enviarComando('0');// Envia comando de desligar para o Arduino
        }
    };

    /*Se o modo automático está desligado, ligamos e enviamos isso para o 
      Arduino com o caractere 'A'.
    */
    const ativarModoAutomatico = () => {
        if (!modoAutomatico) {
            setModoAutomatico(true);
            setIrrigando(false); // Se voltou pro automático, limpa o estado manual de irrigação
            BluetoothService.enviarComando('A');
        }
    };

    return (
        <View style={styles.container}>
            <Text style={styles.titulo}>UMIDADE: {umidade}</Text>
            <Text style={styles.titulo}>ESTADO UMIDADE: {estadoUmidade}</Text>
            <Text style={styles.titulo}>TEMPERATURA: {temperatura}</Text>
            <Text style={styles.titulo}>ESTADO TEMPERATURA: {estadoTemperatura}</Text>
            <Text style={styles.subtitulo}>MODO: {modoAutomatico ? "Automático" : "Manual"}</Text>

            <TouchableOpacity style={styles.botao} onPress={irrigacao}>
                <Text style={styles.textoBotao}>
                    {irrigando ? "PARAR IRRIGAÇÃO" : "IRRIGAR"}
                </Text>
            </TouchableOpacity>

            {!modoAutomatico && (
                <TouchableOpacity style={[styles.botao, styles.botaoAuto]} onPress={ativarModoAutomatico}>
                    <Text style={styles.textoBotao}>VOLTAR PARA AUTOMÁTICO</Text>
                </TouchableOpacity>
            )}
        </View>
    );
}

const styles = StyleSheet.create({
    container: { flex: 1, justifyContent: 'center', alignItems: 'center', backgroundColor: '#F5FCFF' },
    titulo: { fontSize: 20, fontWeight: 'bold', marginVertical: 10 },
    subtitulo: { fontSize: 16, color: '#666', marginBottom: 20 },
    botao: { backgroundColor: '#007AFF', padding: 15, borderRadius: 8, marginVertical: 10, width: 250, alignItems: 'center' },
    botaoAuto: { backgroundColor: '#4CD964' },
    textoBotao: { color: 'white', fontWeight: 'bold' }
});
