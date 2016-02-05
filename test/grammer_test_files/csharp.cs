using System;

class Program
{
    static void Main()
    {
        var dt = new System.Data.DataTable();
        dt.Columns.Add("ID", typeof(String));
        dt.Columns.Add("Val", typeof(int));

        dt.Rows.Add("1", 10);
        dt.Rows.Add("2", 20);
        dt.Rows.Add("3", 10);
        dt.Rows.Add("4", 20);

        System.Data.DataTableReader dr = dt.CreateDataReader();

        for (int I = 0; I < dr.FieldCount; I++) {
            Console.Write(dr.GetName(I) + ",");
        }
        Console.WriteLine();

        while (dr.Read()){
            for (int I = 0; I < dr.FieldCount; I++) {
                Console.Write(dr[dr.GetName(I)] + ",");
            }
            Console.WriteLine();
        }
    }
}