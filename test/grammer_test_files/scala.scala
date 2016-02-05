//【Scala】

import java.io.File
import java.io.FileReader
import java.io.BufferedReader

class Foo {
    def read() {
        val reader = new BufferedReader(new FileReader(new File("temp.txt")))
        try {
            var line : String = null
            while ({ line = reader.readLine; line != null }) {
                println(line)
            }
        } finally {
            reader.close
        }
    }
}