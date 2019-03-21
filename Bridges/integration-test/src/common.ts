import * as vm from "azure-devops-node-api";
//import * as lim from "azure-devops-node-api/interfaces/LocationsInterfaces";

export async function getWebApi(): Promise<vm.WebApi> {
    let serverUrl = "https://bentleycs.visualstudio.com/";
    return await getApi(serverUrl);
}

export async function getApi(serverUrl: string): Promise<vm.WebApi> {
    return new Promise<vm.WebApi>(async (resolve, reject) => {
        try {
            let token = "waooyl6bkhodyzbva4yhkn5guq2gsw7gll64j4eciwb34z22mdvq";
            let authHandler = vm.getPersonalAccessTokenHandler(token);
            let option = undefined;

            // The following sample is for testing proxy
            // option = {
            //     proxy: {
            //         proxyUrl: "http://127.0.0.1:8888"
            //         // proxyUsername: "1",
            //         // proxyPassword: "1",
            //         // proxyBypassHosts: [
            //         //     "github\.com"
            //         // ],
            //     },
            //     ignoreSslError: true
            // };

            // The following sample is for testing cert
            // option = {
            //     cert: {
            //         caFile: "E:\\certutil\\doctest\\ca2.pem",
            //         certFile: "E:\\certutil\\doctest\\client-cert2.pem",
            //         keyFile: "E:\\certutil\\doctest\\client-cert-key2.pem",
            //         passphrase: "test123",
            //     },
            // };

            let vsts: vm.WebApi = new vm.WebApi(serverUrl, authHandler, option);
            //let connData: lim.ConnectionData = await vsts.connect();
            //console.log(`Hello ${connData.authenticatedUser.providerDisplayName}`);
            resolve(vsts);
        }
        catch (err) {
            reject(err);
        }
    });
}

export function getProject(): string {
    return "iModelTechnologies";
}

export function banner(title: string): void {
    console.log("=======================================");
    console.log(`\t${title}`);
    console.log("=======================================");
}

export function heading(title: string): void {
    console.log();
    console.log(`> ${title}`);
}