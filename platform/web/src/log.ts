export class Logger {
  public static showDebugLog: boolean = false;

  public static info(msg: string, ...optionalParams: any[]) {
    if (Logger.showDebugLog) {
      console.log(`animax: ${msg}`, ...optionalParams);
    }
  }

  public static warn(msg: string, ...optionalParams: any[]) {
    if (Logger.showDebugLog) {
      console.warn(`animax: ${msg}`, ...optionalParams);
    }
  }

  public static error(msg: string, ...optionalParams: any[]) {
    console.error(`animax: ${msg}`, ...optionalParams);
  }
}
