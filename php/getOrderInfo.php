<?php 
    $con = mysqli_connect("localhost","root","sqlrootpw","db");
    if(!$con)
    {
        echo "MySQL 접속 에러 : ";
        echo mysqli_connect_error();
        exit();
    }
    mysqli_query("set names utf8");

    $data = file_get_contents('php://input');
    $arr = json_decode($data, TRUE);
    $orderNum = $arr['orderNum'];
    $result = mysqli_query($con, "SELECT * FROM order_info WHERE num = $orderNum");
     $response = array();
    if( $row = mysqli_fetch_array($result))
    {
         $response["success"] = "1";
         $response["password"] = $row["password"];
         $response["btime"] = substr($row["btime"], 11, 8);
         echo json_encode($response);    
    }
    else
    {  
        $response["success"] = "0";
        echo json_encode($response);
    }
?>